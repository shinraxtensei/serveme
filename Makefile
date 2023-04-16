
GREEN = \033[0;32m
RED = \033[0;31m
YELLOW = \033[0;33m
BLUE = \033[0;34m
PURPLE = \033[0;35m
CYAN = \033[0;36m
WHITE = \033[0;37m

NAME = servme

SRCS = srcs/cgi.cpp srcs/delete.cpp srcs/http.cpp srcs/post.cpp srcs/response.cpp \
		srcs/client.cpp srcs/generateError.cpp srcs/lexer.cpp srcs/request.cpp srcs/servme.cpp\
		srcs/core.cpp srcs/get.cpp srcs/parser.cpp srcs/responeUtilities.cpp srcs/socketWrapper.cpp servme.cpp

HEADERS = inc/cgi.hpp inc/core.hpp inc/macros.hpp inc/servme.hpp\
			inc/client.hpp inc/lexer.hpp inc/parser.hpp inc/socketWrapper.hpp



OBJ_DIR = obj/
OBJS = $(addprefix $(OBJ_DIR), $(SRCS:.cpp=.o))

CC = c++

CFLAGS = -Wall -Wextra -Werror -std=c++98 -fsanitize=address  -g


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
	@rm -rf $(OBJ_DIR) parse_tree.dot tmp/*.tmp

fclean: clean
	@rm -f $(NAME)

re : fclean all

.PHONY: all clean fclean re