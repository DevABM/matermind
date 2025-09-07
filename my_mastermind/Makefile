CC = gcc
CFLAGS = -Wall -Wextra -Werror
NAME = my_mastermind
SRC = my_mastermind.c

all: $(NAME)

$(NAME): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(NAME)

clean:
	rm -f $(NAME)

fclean: clean

re: fclean all
