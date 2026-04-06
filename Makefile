# ===========================
#        PROJECT
# ===========================
NAME        := webserv
CXX         := c++
CXXFLAGS    := -Wall -Wextra -Werror -std=c++98
INCLUDES    := -Iincludes
SRC_DIR     := srcs
OBJ_DIR     := obj
# ===========================
#        SOURCES
# ===========================
SRC := \
    srcs/main.cpp \
    srcs/server/client.cpp \
    srcs/server/server.cpp \
    srcs/server/server_client.cpp \
    srcs/server/server_cookies.cpp \
    srcs/server/server_io.cpp \
    srcs/server/server_setup.cpp \
    srcs/server/RequestParser.cpp \
    srcs/http/HttpRequest.cpp \
    srcs/http/HttpResponse.cpp \
    srcs/server/handleRequest.cpp \
    srcs/methods/method_get.cpp \
    srcs/methods/method_post.cpp \
    srcs/methods/method_delete.cpp \
    srcs/config/tokenizer.cpp \
    srcs/config/parser.cpp \
    srcs/config/file_reader.cpp \
    srcs/config/directives.cpp \
    srcs/cgi/cgi.cpp \
    srcs/cgi/cgiParser.cpp \
    srcs/http/http_helpers.cpp

OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
TOTAL := $(words $(SRC))
COUNT := 0
# ===========================
#        COLORS
# ===========================
RESET   := \033[0m
GREEN   := \033[32m
CYAN    := \033[36m
YELLOW  := \033[33m
# ===========================
#        RULES
# ===========================
all: $(NAME)

$(NAME): $(OBJ)
	@echo "\n$(GREEN)[LINK]$(RESET) $(NAME)"
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
	@echo "$(GREEN)✔ Build successful$(RESET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(eval COUNT=$(shell echo $$(($(COUNT)+1))))
	@printf "$(CYAN)[%2d/%2d]$(RESET) %-35s\r" $(COUNT) $(TOTAL) $
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR) $(sort $(dir $(OBJ)))

clean:
	@rm -rf $(OBJ_DIR)
	@echo "$(YELLOW)✘ Object files removed$(RESET)"

fclean: clean
	@rm -f $(NAME)
	@echo "$(YELLOW)✘ Binary removed$(RESET)"

re: fclean all

.PHONY: all clean fclean re