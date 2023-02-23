NAME = servme

SRCS = parser/parser.cpp parsing.cpp

OBJS = $(SRCS:.cpp=.o)

CC = c++

CFLAGS = -Wall -Wextra -Werror -std=c++17 -fsanitize=address


all: $(NAME)

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJS)
	@printf "\n\t\033[1;34mCompilation successful\033[0m\n\n"



%.o: %.cpp 
	@$(CC) $(CFLAGS) -c $< -o $@

run: $(NAME)
	@ ./$(NAME)

clean:
	@rm -f $(OBJS) 

fclean:
	@rm -f $(NAME) $(OBJS) 

re : fclean all

.PHONY: all clean fclean re