#include "snake.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include "tensorflow/cc/framework/gradients.h"

Snake::Snake(const int& grid_side_size, const unsigned int& layer1_size, const unsigned int& layer2_size)
  : grid_side_size(grid_side_size),
    head_x(grid_side_size / 2),
    head_y(grid_side_size / 2),
    position{{static_cast<int>(head_x), static_cast<int>(head_y)}, std::vector<SDL_Point>{}},
    vision{{(grid_side_size - 1) / 2, grid_side_size - 1}, 
      {(grid_side_size - 1) / 2, grid_side_size - 2}, 
      {grid_side_size, grid_side_size},
      {grid_side_size, grid_side_size}},
    root{Scope::NewRootScope()},
    advance_input{Placeholder(root, DT_FLOAT)},
    turn_right_input{Placeholder(root, DT_FLOAT)},
    turn_left_input{Placeholder(root, DT_FLOAT)},
    filter_input{Placeholder(root, DT_FLOAT)},
    filter_state_input{Placeholder(root, DT_FLOAT)},
    mlpOutput{1,3},
    mlpLayer1Size{layer1_size},
    mlpLayer2Size{layer2_size},
    flatWeightsLength{(grid_side_size*grid_side_size + 1)*mlpLayer1Size + (mlpLayer1Size + 1)*mlpLayer2Size + (mlpLayer2Size + 1)*mlpOutputLayerSize},
    brain{flatWeightsLength, 50, 10, 0.03} {

  // Construct the advance operation matricial coefficient
  Matrix advance_coeff(grid_side_size, grid_side_size);
  int one_col = grid_side_size - 1; // Index of the column that shall have the value of 1 (one).
  for(int row = 0; row < grid_side_size; row++) {
    advance_coeff(row, one_col) = 1;
    one_col = (one_col + 1) % grid_side_size;
  }
  advance_op = MatMul(root, advance_coeff.GetTensor(), advance_input);

  // Construct turn direction matricial operator A
  Matrix turn_coeff_a(grid_side_size, grid_side_size);
  one_col = vision.head.x; // Index of the column that shall have the value of 1 (one).
  for(int row = vision.head.y; row >= 0; row--) {
    turn_coeff_a(row, one_col) = 1;
    one_col = (one_col + 1) % grid_side_size;
  }

  // Construct turn direction matricial operator B
  Matrix turn_coeff_b(grid_side_size, grid_side_size);
  one_col = vision.head.x; // Index of the column that shall have the value of 1 (one).
  for(int row = vision.head.y; row >= 0; row--) {
    turn_coeff_b(row, one_col) = 1;
    one_col = (one_col - 1 + grid_side_size) % grid_side_size;
  }

  // Initialize turn right and turn left operators
  turn_right_op = MatMul(root, turn_coeff_a.GetTensor(), MatMul(root, turn_right_input, turn_coeff_b.GetTensor(), {true, false}));
  turn_left_op = MatMul(root, turn_coeff_b.GetTensor(), MatMul(root, turn_left_input, turn_coeff_a.GetTensor(), {true, false}));
  
  // Construct first-order filter operator
  float filter_const = 0.1; //0.2; // Between 0 and 1
  filter_op = Add(root, Multiply(root, filter_const, filter_input), Multiply(root, 1-filter_const, filter_state_input));

  // Initialize Tensorflow session
  session = std::unique_ptr<ClientSession>(new ClientSession(root));

  // Create and initialize MLP neural network for snake's decision model
  Status s = CreateGraphForMLP();
  TF_CHECK_OK(s);
  s = CreateOptimizationGraph(0.001f); //(0.0001f); //input is learning rate
  TF_CHECK_OK(s);
  //Run inititialization
  s = Initialize();
  TF_CHECK_OK(s);

  #if DEBUG_MODE
    std::cout << "Advance operator:" << std::endl;
    std::cout << advance_coeff << std::endl;
    std::cout << "Snake view (before):" << std::endl;
    std::cout << vision.world << std::endl;

    Status scope_status = root.status();
    TF_CHECK_OK(scope_status);
  #endif

  #if DEBUG_MODE
    std::cout << "Snake object created" << std::endl;
  #endif
}

void Snake::Move() {
  #if DEBUG_MODE
    std::cout << "Snake began to move..." << std::endl;
  #endif

  SDL_Point prev_cell(position.head);  // We first capture the head's cell before updating.
  MoveHead();

  if (position.head.x != prev_cell.x || position.head.y != prev_cell.y) {
    // Snake head has moved to a new world grid tile.
    event = Event::NewTile;

    // Senses the front tile content and raises any event (e.g. eating, collision, etc.)
    SenseFrontTile();

    // Filter world view in order to use it as input to the snake's learn and decision model.
    FilterWorldView();

    // Make snake AI model learn, based on the result of its latest action.
    // This learning process shall occur even when the player is controlling the snake, so that the
    // AI may also learn from observing the player himself.
    Learn(prev_cell);

    // Advance world view in one tile forward and update body
    UpdateBodyAndWorldView(prev_cell);

    // Runs AI model for snake's next action.
    DefineAction();

  } else {
    // Snake head is still in the same world grid tile.
    event = Event::SameTile;
  }

  #if DEBUG_MODE
    std::cout << "Snake moved!" << std::endl;
  #endif
}

void Snake::FilterWorldView() {
  std::vector<Tensor> output;

  // Input the current world view matrix and previous filtered view to the first-order filter operator.
  TF_CHECK_OK(session->Run({{filter_input, vision.world.GetTensor()}, {filter_state_input, vision.worldFilt.GetTensor()}}, {filter_op}, &output));

  // Update filtered world view matrix with the result of the filter operation.
  vision.worldFilt = std::move(output[0]);
}

void Snake::MoveHead() {
  switch (direction) {
    case Direction::Up:
      head_y -= speed;
      break;

    case Direction::Down:
      head_y += speed;
      break;

    case Direction::Left:
      head_x -= speed;
      break;

    case Direction::Right:
      head_x += speed;
      break;
  }

  // Wrap the Snake around to the beginning if going off of the screen.
  // TODO: repeated operation
  head_x = fmod(head_x + grid_side_size, grid_side_size);
  head_y = fmod(head_y + grid_side_size, grid_side_size);

  position.head.x = static_cast<int>(head_x);
  position.head.y = static_cast<int>(head_y);

  #if DEBUG_MODE
    std::cout << "Snake head moved!" << std::endl;
  #endif
}

void Snake::SeeFood(const SDL_Point& food_position) {
  // Update global food position reference in Snake object.
  position.food = food_position;

  // Update in snake view grid.
  SetWorldViewElement(food_position, Snake::WorldElement::Food);
}

void Snake::SetWorldViewElement(const SDL_Point& position, const Snake::WorldElement& new_element) {
  // Convert ṕosition from player's to snake's perspective
  SDL_Point snake_view_position = ToSnakeVision(position);

  // Update in snake view grid
  vision.world(snake_view_position.y, snake_view_position.x) = static_cast<int>(new_element);
}

void Snake::ProcessUserCommand(const Controller::UserCommand& command) {
  if(command == Controller::UserCommand::ToggleAutoMode) ToggleAutoMode();
  else if(!automode) {
    // If auto mode is on, only the auto mode toggling command is available, and all other are ignored.
    switch(command) {
      case Controller::UserCommand::GoUp:
        if (direction == Direction::Left) Act(Action::MoveRight);
        else if (direction == Direction::Right) Act(Action::MoveLeft);
        else Act(Action::MoveFwd);
        break;
      case Controller::UserCommand::GoDown:
        if (direction == Direction::Left) Act(Action::MoveLeft);
        else if (direction == Direction::Right) Act(Action::MoveRight);
        else Act(Action::MoveFwd);
        break;
      case Controller::UserCommand::GoLeft:
        if (direction == Direction::Up) Act(Action::MoveLeft);
        else if (direction == Direction::Down) Act(Action::MoveRight);
        else Act(Action::MoveFwd);
        break;
      case Controller::UserCommand::GoRight:
        if (direction == Direction::Up) Act(Action::MoveRight);
        else if (direction == Direction::Down) Act(Action::MoveLeft);
        else Act(Action::MoveFwd);
        break;
      default:
        // UserCommand::None (no command issued by user)
        break;
    }
  }
}

void Snake::Act(const Action& input) {
  action = input;
  if(action == Action::MoveFwd) {
    // Do nothing
  } else {
    if(action == Action::MoveLeft) {
      direction = GetLeftOf(direction);
    } else {
      // Action::MoveRight
      direction = GetRightOf(direction);
    }

    // Update the snake front vision to the new perspective (due to the new direction).
    TurnEyes();
  }
}

SDL_Point Snake::ToSnakeVision(const SDL_Point& point) const {
  SDL_Point distance_to_head;
  distance_to_head.x = point.x - position.head.x;
  distance_to_head.y = point.y - position.head.y;

  /* Convert to a position relative to the snake's head, from its perspective. */
  SDL_Point new_point;
  switch (direction) {
    int aux;
    case Direction::Down:
      distance_to_head.x = -distance_to_head.x;
      distance_to_head.y = -distance_to_head.y;
      break;
    case Direction::Right:
      aux = distance_to_head.x;
      distance_to_head.x = distance_to_head.y;
      distance_to_head.y = -aux;
      break;
    case Direction::Left:
      aux = distance_to_head.x;
      distance_to_head.x = -distance_to_head.y;
      distance_to_head.y = aux;
      break;
    default: // Direction::Up
      /* No mapping factor needed */
      break;
  }
  // TODO: repeated operation
  new_point.x = (distance_to_head.x + vision.head.x + grid_side_size) % grid_side_size;
  new_point.y = (distance_to_head.y + vision.head.y + grid_side_size) % grid_side_size;
  
  return new_point;
}

// TODO: desvincular Init method da inicialização da comida na visão da cobra, e incluir Init no construtor tambem
void Snake::Init(const SDL_Point& food_position) {
  // Reset all snake parameters.
  alive = true;
  position.body.clear();
  size = 1;
  speed = INITIAL_SNAKE_SPEED;
  event = Event::SameTile;
  action = Action::MoveFwd;
  vision.world.Reset();
  vision.worldFilt.Reset();

  // Initialize the snake's world view based on the food and its body positions.
  // Initialize food tile.
  SeeFood(food_position);

  // Initialize snake head tile.
  SetWorldViewElement(position.head, Snake::WorldElement::Head);

  // Initialize snake body tiles.
  for(const SDL_Point& body_part : position.body) {
    SetWorldViewElement(body_part, Snake::WorldElement::Body);
  }

  #if DEBUG_MODE
    std::cout << "Snake initiated!" << std::endl;
  #endif
}

void Snake::Learn(const SDL_Point& prev_head_position) {
  /* TODO: update the snake's AI model based on Snake::event */
  
  // Construct the current feedback vector, based on the current event.
  bool strengthenAction = false;
  bool weakenAction = false;
  bool sevWeakenAction = false;
  switch(event) {
    case Event::Collided:
      sevWeakenAction = true;
      break;
    case Event::Ate:
      strengthenAction = true;
      break;
    default: // Event::NewTile
      // Takes into consideration the distance to the food
      int prev_distance = DistanceToFood(prev_head_position);
      int distance = DistanceToFood(position.head);
      //if (distance < prev_distance) strengthenAction = true;
      //else if (distance > prev_distance) weakenAction = true;
      //else weakenAction = true;
      //std::cout << std::endl << "Previous distance to food: " << prev_distance << std::endl;
      //std::cout << "Current distance to food: " << distance << std::endl;
      break; 
  }

  if (strengthenAction || weakenAction || sevWeakenAction) {
    float actionValue;
    float actionValueOthers;
    if (strengthenAction) {
      actionValue = 1;
      actionValueOthers = 0;
      std::cout << "Strengthen Action!" << std::endl;
    } else {
      actionValue = 0;
      actionValueOthers = 1;
      std::cout << "Weaken Action..." << std::endl;
    }

    Matrix feedback(1, 3);
    switch(action) {
      case Action::MoveLeft:
        if (strengthenAction) {
          feedback(0,0) = 1;
          feedback(0,1) = 0;
          feedback(0,2) = 0;
        } else if (weakenAction) {
          feedback(0,0) = 0;

          float total_weight = mlpOutput(0,1) + mlpOutput(0,2);
          float probability = mlpOutput(0,1) / total_weight;

          float number = random_direction_distribution(generator);

          if (number < probability) {
            feedback(0,1) = 1;
            feedback(0,2) = 0;
          } else {
            feedback(0,1) = 0;
            feedback(0,2) = 1;
          }

          //feedback(0,1) = actionValueOthers; //mlpOutput(0,1);
          //feedback(0,2) = actionValueOthers; //mlpOutput(0,2);
        } else {
          feedback(0,0) = 0;
          feedback(0,1) = 1;
          feedback(0,2) = 1;
        }
        
        break;
      case Action::MoveRight:
        if (strengthenAction) {
          feedback(0,0) = 0;
          feedback(0,1) = 0;
          feedback(0,2) = 1;
        } else if (weakenAction) {
          feedback(0,2) = 0;

          float total_weight = mlpOutput(0,0) + mlpOutput(0,1);
          float probability = mlpOutput(0,0) / total_weight;

          float number = random_direction_distribution(generator);

          if (number < probability) {
            feedback(0,0) = 1;
            feedback(0,1) = 0;
          } else {
            feedback(0,0) = 0;
            feedback(0,1) = 1;
          }
        } else {
          feedback(0,0) = 1;
          feedback(0,1) = 1;
          feedback(0,2) = 0;
        }
        /*
        feedback(0,0) = actionValueOthers; //mlpOutput(0,0);
        feedback(0,1) = actionValueOthers; //mlpOutput(0,1);
        feedback(0,2) = actionValue;
        */
        break;
      default: // Action::MoveFwd
        if (strengthenAction) {
          feedback(0,0) = 0;
          feedback(0,1) = 1;
          feedback(0,2) = 0;
        } else if (weakenAction) {
          feedback(0,1) = 0;

          float total_weight = mlpOutput(0,0) + mlpOutput(0,2);
          float probability = mlpOutput(0,0) / total_weight;

          float number = random_direction_distribution(generator);

          if (number < probability) {
            feedback(0,0) = 1;
            feedback(0,2) = 0;
          } else {
            feedback(0,0) = 0;
            feedback(0,2) = 1;
          }
        } else {
          feedback(0,0) = 1;
          feedback(0,1) = 0;
          feedback(0,2) = 1;
        }
        /*
        feedback(0,0) = actionValueOthers; //mlpOutput(0,0);
        feedback(0,1) = actionValue;
        feedback(0,2) = actionValueOthers; //mlpOutput(0,2);
        */
        break;
    }

    // Input the current world view matrix to the decision MLP, together with the current event,
    // in order to train it.
    //TF_CHECK_OK(TrainMLP(vision.worldFilt.GetTensor(), feedback.GetTensor()));
  }
}

int Snake::DistanceToFood(const SDL_Point& head_position) {
  // Calculate "city block" distance from snake head to food.
  int distance = 0;
  distance += std::min(abs(head_position.x - position.food.x), 
    (head_position.x < position.food.x)? (grid_side_size - position.food.x + head_position.x) : 
      (grid_side_size - head_position.x + position.food.x));
  distance += std::min(abs(head_position.y - position.food.y), 
    (head_position.y < position.food.y)? (grid_side_size - position.food.y + head_position.y) : 
      (grid_side_size - head_position.y + position.food.y));
  return distance;
}

void Snake::DefineAction() {
  /* TODO: run the snake's AI model based on its world view and update Snake::action */
  
  // Input the current world view matrix to the decision MLP, together with the current event,
  // and get output.
  TF_CHECK_OK(RunMLP(vision.worldFilt.GetTensor()));

  if (automode) {
    // If auto mode is active, sets the snake's next action based on the AI model output.
    float total_weight = mlpOutput(0,0) + mlpOutput(0,1) + mlpOutput(0,2);
    float probabilityLeft = mlpOutput(0,0) / total_weight;
    float probabilityRight = mlpOutput(0,2) / total_weight;

    // Random decision model
    float number = random_direction_distribution(generator);

    if (number < probabilityLeft) {
      /* Chance to move left from current direction */
      Act(Action::MoveLeft);
      std::cout << "Move Left!" << std::endl;
    } else if (number < probabilityLeft + probabilityRight) {
      /* Chance to move right from current direction */
      Act(Action::MoveRight);
      std::cout << "Move Right!" << std::endl;
    } else {
      /* Chance to maintain current direction and move forward */
      Act(Action::MoveFwd);
      std::cout << "Move Forward!" << std::endl;
    }
  }
}

Snake::Direction Snake::GetLeftOf(const Snake::Direction& reference) {
  return static_cast<Snake::Direction>((static_cast<uint8_t>(reference) + 3) % 4); 
}

Snake::Direction Snake::GetRightOf(const Snake::Direction& reference) { 
  return static_cast<Snake::Direction>((static_cast<uint8_t>(reference) + 1) % 4); 
}

void Snake::SenseFrontTile() {
  #if DEBUG_MODE
    std::cout << "Snake began sensing front tile..." << std::endl;
  #endif

  // Check if the tile where the snake head moved to contain a body part or food, and raise event.
  if (GetWorldViewElement({vision.front_tile}) == WorldElement::Body) {
    event = Event::Collided;
    alive = false;
  } else if (GetWorldViewElement(vision.front_tile) == WorldElement::Food) {
    event = Event::Ate;
    // Increase snake's size and speed.
    size++;
    speed += 0.02;
  }

  #if DEBUG_MODE
    std::cout << "Snake sensed front tile!" << std::endl;
  #endif
}

void Snake::UpdateBodyAndWorldView(const SDL_Point& prev_head_position) {
  #if DEBUG_MODE
    std::cout << "Snake began advancing world view..." << std::endl;
  #endif

  // Perform matricial operation and advance world view matrix in one tile ahead.
  std::vector<Tensor> output;

  #if DEBUG_MODE
    Status scope_status = root.status();
    TF_CHECK_OK(scope_status);

    std::cout << "Operation began running..." << std::endl;
    std::cout << "Snake view (before):" << std::endl;
    std::cout << vision.world << std::endl;
  #endif

  // Input the current world view matrix to the advance operator.
  TF_CHECK_OK(session->Run({{advance_input, vision.world.GetTensor()}}, {advance_op}, &output));

  // Update world view matrix with the result of the advance operation.
  vision.world = std::move(output[0]);
  
  #if DEBUG_MODE
    std::cout << "Operation ran..." << std::endl;
    std::cout << "Snake view (after):" << std::endl;
    std::cout << vision.world << std::endl;
  #endif

  // Next update the current head position as a head in the snake world view.
  SetWorldViewElement(position.head, WorldElement::Head);

  // Update the snake body location, if it has one.
  if (size > 1) {
    // If the snake has a body, add previous head location to the body vector.
    position.body.push_back(prev_head_position);
    // Also update this position as a body part, in the world view.
    SetWorldViewElement(prev_head_position, Snake::WorldElement::Body);

    // Next, in case the snake didn't eat, move the oldest body vector item (i.e. the snake's tail).
    if (event != Snake::Event::Ate) {
      // Remove the tail from the snake vision of the world grid.
      SetWorldViewElement(position.body.front(), Snake::WorldElement::None);

      // Remove the tail from the body vector.
      position.body.erase(position.body.begin());
    }
  } else {
    // Otherwise, if the snake has only a head, clear the previous head position in the world view.
    SetWorldViewElement(prev_head_position, Snake::WorldElement::None);
  }

  #if DEBUG_MODE
    std::cout << "Snake advanced world view!" << std::endl;
  #endif
}

void Snake::TurnEyes() {
  #if DEBUG_MODE
    std::cout << "Snake began turning eyes..." << std::endl;
  #endif

  std::vector<Tensor> output;
  std::vector<Tensor> outputFilt;

  if(action == Action::MoveLeft) {
    // Rotate snake vision grid 90 degrees to the right
    TF_CHECK_OK(session->Run({{turn_left_input, vision.world.GetTensor()}}, {turn_left_op}, &output));
    TF_CHECK_OK(session->Run({{turn_left_input, vision.worldFilt.GetTensor()}}, {turn_left_op}, &outputFilt));
  } else {
    // Rotate snake vision grid 90 degrees to the left
    TF_CHECK_OK(session->Run({{turn_right_input, vision.world.GetTensor()}}, {turn_right_op}, &output));
    TF_CHECK_OK(session->Run({{turn_right_input, vision.worldFilt.GetTensor()}}, {turn_right_op}, &outputFilt));
  }

  // Update vision matrix with the result of the operation.
  vision.world = std::move(output[0]);
  vision.worldFilt = std::move(outputFilt[0]);

  #if DEBUG_MODE
    std::cout << "Snake turned eyes!" << std::endl;
  #endif
}

Status Snake::CreateGraphForMLP()
{
    //Input vector (representing the snake world view) is of size: grid_side_size x grid_side_size.
    input_view_var = Placeholder(root.WithOpName("input"), DT_FLOAT);

    //Flatten
    Scope flatten = root.NewSubScope("flat_layer");
    int flat_len = grid_side_size * grid_side_size;
    auto flat = Reshape(flatten, input_view_var, {-1, flat_len});
    
    //Dense No 1
    int in_units = flat_len;
    int out_units = mlpLayer1Size;
    Scope scope_dense1 = root.NewSubScope("Dense1_layer");
    auto dense1 = AddDenseLayer("1", scope_dense1, in_units, out_units, true, flat);

    //Dense No 2
    in_units = out_units;
    out_units = mlpLayer2Size;
    Scope scope_dense2 = root.NewSubScope("Dense2_layer");
    auto dense2 = AddDenseLayer("2", scope_dense2, in_units, out_units, true, dense1);
    
    //Dense No 3
    in_units = out_units;
    out_units = mlpOutputLayerSize;
    Scope scope_dense3 = root.NewSubScope("Dense3_layer");
    auto logits = AddDenseLayer("3", scope_dense3, in_units, out_units, false, dense2);

    out_classification = Sigmoid(root.WithOpName("Output_Classes"), logits);
    return root.status();
}

Input Snake::AddDenseLayer(string idx, Scope scope, int in_units, int out_units, bool bActivation, Input input)
{
    TensorShape sp = {in_units, out_units};
    m_vars["W"+idx] = Variable(scope.WithOpName("W"), sp, DT_FLOAT);
    m_shapes["W"+idx] = sp;
    m_assigns["W"+idx+"_assign"] = Assign(scope.WithOpName("W_assign"), m_vars["W"+idx], XavierInit(scope, in_units, out_units));
    sp = {out_units};
    m_vars["B"+idx] = Variable(scope.WithOpName("B"), sp, DT_FLOAT);
    m_shapes["B"+idx] = sp;
    m_assigns["B"+idx+"_assign"] = Assign(scope.WithOpName("B_assign"), m_vars["B"+idx], Input::Initializer(0.f, sp));
    auto dense = Add(scope.WithOpName("Dense_b"), MatMul(scope.WithOpName("Dense_w"), input, m_vars["W"+idx]), m_vars["B"+idx]);
    if(bActivation)
        return Relu(scope.WithOpName("Relu"), dense);
    else
        return dense;
}

Status Snake::CreateOptimizationGraph(float learning_rate)
{
    input_label_var = Placeholder(root.WithOpName("inputL"), DT_FLOAT);
    Scope scope_loss = root.NewSubScope("Loss_scope");
    out_loss_var = SquaredDifference(scope_loss.WithOpName("Loss"), out_classification, input_label_var);
    TF_CHECK_OK(scope_loss.status());
    std::vector<Output> weights_biases;
    for(std::pair<string, Output> i: m_vars)
        weights_biases.push_back(i.second);
    std::vector<Output> grad_outputs;
    TF_CHECK_OK(AddSymbolicGradients(root, {out_loss_var}, weights_biases, &grad_outputs));
    int index = 0;
    for(std::pair<string, Output> i: m_vars)
    {
        //Applying Adam
        string s_index = std::to_string(index);
        auto m_var = Variable(root, m_shapes[i.first], DT_FLOAT);
        auto v_var = Variable(root, m_shapes[i.first], DT_FLOAT);
        m_assigns["m_assign"+s_index] = Assign(root, m_var, Input::Initializer(0.f, m_shapes[i.first]));
        m_assigns["v_assign"+s_index] = Assign(root, v_var, Input::Initializer(0.f, m_shapes[i.first]));

        auto adam = ApplyAdam(root, i.second, m_var, v_var, 0.f, 0.f, learning_rate, 0.9f, 0.999f, 0.00000001f, {grad_outputs[index]});
        v_out_grads.push_back(adam.operation);
        index++;
    }
    return root.status();
}

Status Snake::Initialize()
{
    if(!root.ok())
        return root.status();
    
    std::vector<Output> ops_to_run;
    for(std::pair<string, Output> i: m_assigns)
        ops_to_run.push_back(i.second);
    TF_CHECK_OK(session->Run(ops_to_run, nullptr));
    /*
    GraphDef graph;
    TF_RETURN_IF_ERROR(t_root.ToGraphDef(&graph));
    SummaryWriterInterface* w;
    TF_CHECK_OK(CreateSummaryFileWriter(1, 0, "/Users/bennyfriedman/Code/TF2example/TF2example/graphs", ".cnn-graph", Env::Default(), &w));
    TF_CHECK_OK(w->WriteGraph(0, make_unique<GraphDef>(graph)));
    */
    return Status::OK();
}

Status Snake::RunMLP(const Tensor& view)
{
    if(!root.ok())
        return root.status();
    
    std::vector<Tensor> out_tensors;
    //Inputs: image, drop rate 1 and skip drop.
    TF_CHECK_OK(session->Run({{input_view_var, view}}, {out_classification}, &out_tensors));
    mlpOutput = std::move(out_tensors[0]);
    std::cout << "MLP prediction output: " << mlpOutput << std::endl;
    return Status::OK();
}

Input Snake::XavierInit(Scope scope, int in_chan, int out_chan, int filter_side)
{
    float std;
    Tensor t;
    if(filter_side == 0)
    { //Dense
        std = sqrt(6.f/(in_chan+out_chan));
        Tensor ts(DT_INT64, {2});
        auto v = ts.vec<int64>();
        v(0) = in_chan;
        v(1) = out_chan;
        t = ts;
    }
    else
    { //Conv
        std = sqrt(6.f/(filter_side*filter_side*(in_chan+out_chan)));
        Tensor ts(DT_INT64, {4});
        auto v = ts.vec<int64>();
        v(0) = filter_side;
        v(1) = filter_side;
        v(2) = in_chan;
        v(3) = out_chan;
        t = ts;
    }
    auto rand = RandomUniform(scope, t, DT_FLOAT);
    return Multiply(scope, Sub(scope, rand, 0.5f), std*2.f);
}