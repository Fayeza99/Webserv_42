# Compiler
CXX = c++

# Compiler Flags
CXXFLAGS = -Wall -Wextra -Werror -std=c++17 -Iincludes

# Source Files
SOURCES = $(wildcard srcs/*.cpp srcs/*/*.cpp)

# Object files directory
OBJDIR = obj

# Object files
OBJECTS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SOURCES))

# Executable name
EXEC = webserv

# Default target
all: $(EXEC)

# Linking all the object files to create the executable
$(EXEC): $(OBJECTS)
	$(CXX) -o $(EXEC) $(OBJECTS)

# Compiling each source file into the object directory
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target for removing object files
clean:
	rm -rf $(OBJDIR)

# Clean target for removing executable with the object files
fclean: clean
	rm -rf $(EXEC)

# Target to clean and make everything again
re: fclean all

.PHONY: all clean fclean re
