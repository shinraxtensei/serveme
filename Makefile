NAME = servme

SRCS = srcs/parser.cpp srcs/core.cpp servme.cpp 

OBJ_DIR = obj/
OBJS = $(addprefix $(OBJ_DIR), $(SRCS:.cpp=.o))

CC = c++

CFLAGS = -Wall -Wextra -Werror -std=c++17 -fsanitize=address


all: $(NAME)
	@./$(NAME) 



$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJS)
	@printf "\n\t\033[1;34mCompilation successful\033[0m\n\n"

$(OBJ_DIR)%.o: %.cpp
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@

run: $(NAME)
	@ ./$(NAME)

clean:
	@rm -rf $(OBJ_DIR) output.dot

fclean: clean
	@rm -f $(NAME)

re : fclean all

.PHONY: all clean fclean re