
# Macros
CLR:=clear 
CLN_MSG:=echo "\\nALL TIDY!\\n"


# Directories
CLIENT_FS_DIR:=  ./src/fileserver/client
CLIENT_LOG_DIR:= ./src/logging/client

TEST_SERVER_DIR:= ~/test/server
TEST_CLIENT_DIR:= ~/test/client/download

FILE_SERVER_DIR:= ./src/fileserver/server
LOG_SERVER_DIR:=  ./src/logging/server
SRC_DIR:= ./src
OUTPUT_DIR:= .
LIB_DIR:= ./lib


# Includes
INC_PATH= ./inc


# Objects
CLIENT_FS_OBJ= $(CLIENT_FS_DIR)/interactive_client.o $(CLIENT_FS_DIR)/fileserver_protocolos.o $(CLIENT_FS_DIR)/interactive_funcs.o \
               $(SRC_DIR)/file_operations.o $(SRC_DIR)/fileserver_common.o 

CLIENT_LOG_OBJ= $(CLIENT_LOG_DIR)/multiple_random_clients.o

SERVER_OBJ= $(SRC_DIR)/server.o $(SRC_DIR)/reactor.o $(SRC_DIR)/selector.o $(SRC_DIR)/file_operations.o $(SRC_DIR)/fileserver_common.o \
            $(FILE_SERVER_DIR)/repo_handler.o $(FILE_SERVER_DIR)/service_handler.o $(LOG_SERVER_DIR)/log_handler.o
            
LIB_OBJ= $(LIB_DIR)/lib.a


# Targets
CLIENT_FS=  $(OUTPUT_DIR)/fileserver_client
CLIENT_LOG= $(OUTPUT_DIR)/log_client

SERVER= $(OUTPUT_DIR)/server


# Compilation & Link flags
CC:= g++ 
CPPFLAGS= -g -pedantic -ansi -Wall -I$(INC_PATH) -m32
LDFLAGS:= -g -pthread -m32 -lrt

.PHONY: all
all: $(SERVER) $(CLIENT_FS) $(CLIENT_LOG)

include depends

$(SERVER): $(SERVER_OBJ) $(LIB_OBJ)
	 $(CC) $^ $(LDFLAGS) -o $@

$(CLIENT_FS): $(CLIENT_FS_OBJ)
	 $(CC) $^ $(LDFLAGS) -o $@

$(CLIENT_LOG): $(CLIENT_LOG_OBJ)
	 $(CC) $^ $(LDFLAGS) -o $@

depends:
	$(CC) -I$(INC_PATH) -MM $(SERVER_OBJ:.o=.cpp) $(CLIENT_FS_OBJ:.o=.cpp) $(CLIENT_LOG_OBJ:.o=.cpp) > depends

.PHONY: clean
.SILENT: clean	
clean: ;$(CLR) ;$(CLN_MSG) 
	$(RM) *~ $(SERVER_OBJ) $(SERVER) $(CLIENT_FS_OBJ) $(CLIENT_FS) $(CLIENT_LOG_OBJ) $(CLIENT_LOG) depends logs/*
	$(RM) $(TEST_SERVER_DIR)/* $(TEST_CLIENT_DIR)/* 
