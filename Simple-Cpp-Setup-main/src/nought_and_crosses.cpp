#include "nought_and_crosses.h"

glm::vec2 get_cell_center(Cell cell, int board_size)
{
    return p6::map(glm::vec2{static_cast<float>(cell.x),
                             static_cast<float>(cell.y)},
                   glm::vec2{0.f}, glm::vec2{static_cast<float>(board_size)},
                   glm::vec2{-1.f}, glm::vec2{1.f}) +
           1.f / static_cast<float>(board_size);
}

void draw_cell(Cell cell, int board_size, p6::Context& ctx)
{
    ctx.square(p6::BottomLeftCorner{p6::map(glm::vec2{static_cast<float>(cell.x),
                                                      static_cast<float>(cell.y)},
                                            glm::vec2{0.f}, glm::vec2{static_cast<float>(board_size)},
                                            glm::vec2{-1.f}, glm::vec2{1.f})},
               p6::Radius{1.f / static_cast<float>(board_size)});
}

void draw_board(int size, p6::Context& ctx)
{
    for (int x = 0; x < size; ++x) {
        for (int y = 0; y < size; ++y) {
            draw_cell({x, y}, size, ctx);
        }
    }
}

void draw_nought(Cell cell, int board_size, p6::Context& ctx)
{
    ctx.stroke        = {0, 0, 0};
    ctx.fill          = {0, 0, 0, 0};
    ctx.stroke_weight = 0.4f * 1.f / static_cast<float>(board_size);
    ctx.circle(p6::Center{get_cell_center(cell, board_size)}, p6::Radius{0.9f * 1.f / static_cast<float>(board_size)});
}

void draw_cross(Cell cell, int board_size, p6::Context& ctx)
{
    ctx.stroke          = {0, 0, 0};
    ctx.fill            = {0, 0, 0, 0};
    ctx.stroke_weight   = 0.4f * 1.f / static_cast<float>(board_size);
    const auto center   = p6::Center{get_cell_center(cell, board_size)};
    const auto radii    = p6::Radii{glm::vec2{1.f, 0.2f} * 1.f / static_cast<float>(board_size)};
    const auto rotation = p6::Rotation{0.125_turn};
    ctx.rectangle(center, radii, rotation);
    ctx.rectangle(center, radii, -rotation);
}

template<int size>
void draw_noughts_and_crosses(const Board<size>& board, p6::Context& ctx)
{
    for (int x = 0; x < size; ++x) {
        for (int y = 0; y < size; ++y) {
            const auto cell = board[{x, y}];
            if (cell.has_value()) {
                draw_player(*cell, {x, y}, size, ctx);
            }
        }
    }
}

void change_player(Player& player)
{
    if (player == Player::Noughts) {
        player = Player::Crosses;
    }
    else {
        player = Player::Noughts;
    }
}

std::optional<Cell> cell_hovered(glm::vec2 position, int board_size)
{
    const auto pos = p6::map(position,
                             glm::vec2{-1.f}, glm::vec2{1.f},
                             glm::vec2{0.f}, glm::vec2{static_cast<float>(board_size)});

    const auto cell = Cell{
        static_cast<int>(std::floor(pos.x)),
        static_cast<int>(std::floor(pos.y))};

    if (cell.x >= 0 && cell.x < board_size &&
        cell.y >= 0 && cell.y < board_size) {
        return std::make_optional(cell);
    }
    else {
        return std::nullopt;
    }
}

void draw_player(Player player, Cell cell, int board_size, p6::Context& ctx)
{
    if (player == Player::Noughts) {
        draw_nought(cell, board_size, ctx);
    }
    else {
        draw_cross(cell, board_size, ctx);
    }
}

template<int board_size>
void try_to_play(std::optional<Cell> cell_index, Board<board_size>& board, Player& current_player)
{
    if (cell_index.has_value()) {
        const auto cell_is_empty = !board[*cell_index].has_value();
        if (cell_is_empty) {
            board[*cell_index] = current_player;
            change_player(current_player);
        }
    }
}

template<int board_size>
void try_draw_player_on_hovered_cell(Player player, Board<board_size> board, p6::Context& ctx)
{
    const auto hovered_cell = cell_hovered(ctx.mouse(), board_size);
    if (hovered_cell.has_value() && !board[*hovered_cell].has_value()) {
        draw_player(player, *hovered_cell, board_size, ctx);
    }
}

template<int board_size>
bool board_is_full(const Board<board_size>& board)
{
    return std::all_of(board.begin(), board.end(), [](const auto& cell) {
        return cell.has_value();
    });
}

template<int board_size>
std::optional<Player> check_for_winner_on_line(const Board<board_size>& board, std::function<Cell(int)> index_generator)
{
    const bool are_all_equal = [&]() {
        for (int position = 0; position < board_size - 1; ++position) {
            if (board[index_generator(position)] != board[index_generator(position + 1)]) {
                return false;
            }
        }
        return true;
    }();
    if (are_all_equal && board[index_generator(0)].has_value()) {
        return *board[index_generator(0)];
    }
    else {
        return std::nullopt;
    }
}

template<int board_size>
std::optional<Player> check_for_winner(const Board<board_size>& board)
{
    std::optional<Player> winner = std::nullopt;
    // Columns
    for (int x = 0; x < board_size && !winner.has_value(); ++x) {
        winner = check_for_winner_on_line(board, [x](int position) {
            return Cell{x, position};
        });
    }
    // Rows
    for (int y = 0; y < board_size && !winner.has_value(); ++y) {
        winner = check_for_winner_on_line(board, [y](int position) {
            return Cell{position, y};
        });
    }
    // Diagonal
    if (!winner.has_value()) {
        winner = check_for_winner_on_line(board, [](int position) {
            return Cell{position, position};
        });
    }
    // Anti-diagonal
    if (!winner.has_value()) {
        winner = check_for_winner_on_line(board, [](int position) {
            return Cell{position, board_size - position - 1};
        });
    }
    return winner;
}

template<int board_size>
bool game_is_finished(const Board<board_size>& board)
{
    if (const auto winner = check_for_winner(board); winner.has_value()) {
        if (*winner == Player::Noughts) {
            std::cout << "Noughts have won!\n";
        }
        else {
            std::cout << "Crosses have won!\n";
        }
        return true;
    }
    else if (board_is_full(board)) {
        std::cout << "This is a draw!\n";
        return true;
    }
    else {
        return false;
    }
}

void nought_and_crosses()
{
    static constexpr int board_size     = 3;
    auto                 board          = Board<board_size>{};
    auto                 current_player = Player::Crosses;

    auto ctx          = p6::Context{{1000, 1000, "Noughts and Crosses"}};
    ctx.mouse_pressed = [&](p6::MouseButton event) {
        try_to_play(cell_hovered(event.position, board_size), board, current_player);
    };
    ctx.update = [&]() {
        ctx.background({0.85f, 0.55f, 0.19f, 1.f});
        ctx.stroke_weight = 0.01f;
        ctx.stroke        = {1.f, 1.f, 1.f, 1.f};
        ctx.fill          = {0.f, 0.f, 0.f, 0.f};
        draw_board(board_size, ctx);
        draw_noughts_and_crosses(board, ctx);
        try_draw_player_on_hovered_cell(current_player, board, ctx);

        if (game_is_finished(board)) {
            ctx.stop();
        }
    };
    ctx.start();
}
