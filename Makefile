
GREEN = \033[0;32m
RED = \033[0;31m
YELLOW = \033[0;33m
BLUE = \033[0;34m
PURPLE = \033[0;35m
CYAN = \033[0;36m
WHITE = \033[0;37m

NAME = servme

# SRCS = srcs/parser.cpp srcs/core.cpp \
 		# srcs/client.cpp servme.cpp  srcs/generateError.cpp\
		# srcs/lexer.cpp srcs/request.cpp srcs/server.cpp\
		# srcs/socketWrapper.cpp

SRCS = $(shell find srcs -type f -name "*.cpp") servme.cpp


HEADER_DIR = inc/
HEADERS  = $(shell find $(HEADER_DIR) -type f -name "*.hpp")



OBJ_DIR = obj/
OBJS = $(addprefix $(OBJ_DIR), $(SRCS:.cpp=.o))

CC = c++

CFLAGS = -Wall -Wextra -Werror -std=c++17 -fsanitize=address  -g
# CFLAGS = -Weverything


all: $(NAME)




$(NAME): $(HEADERS) $(OBJS) 
	@$(CC) $(CFLAGS) -o $(NAME)  $(OBJS)
	@printf "\n\t\033[1;34mCompilation successful\033[0m\n\n"

$(OBJ_DIR)%.o: %.cpp
	@echo "$(GREEN) Compiling :$(WHITE) $<\r"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@

run: $(NAME)
	@ ./$(NAME)

clean:
	@rm -rf $(OBJ_DIR) parse_tree.dot

fclean: clean
	@rm -f $(NAME)

re : fclean all

.PHONY: all clean fclean re