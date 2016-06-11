CSRCS = tool.cpp shake.cpp slow.cpp
EX1_SRCS = server.cpp
EX2_SRCS = client.cpp
EXT = .cpp
COBJS = $(CSRCS:$(EXT)=.o)
EX1_OBJS = $(EX1_SRCS:$(EXT)=.o)
EX2_OBJS = $(EX2_SRCS:$(EXT)=.o)
EXEC1 = server
EXEC2 = client
CXX = g++
CXXFLAGS = -Wall -std=c++11
DEP = dep

all: $(EXEC1) $(EXEC2)

debug: CXXFLAGS += -D__DEBUG__
debug: all

seq: CXXFLAGS += -D__SEQSTATIC__
seq: all

transeq: CXXFLAGS += -D__TRANSEQ__
transeq: all

$(EXEC1): $(COBJS) $(EX1_OBJS)
		$(CXX) $(CXXFLAGS) -o $@ $(COBJS) $(EX1_OBJS)

$(EXEC2): $(COBJS) $(EX2_OBJS)
		$(CXX) $(CXXFLAGS) -o $@ $(COBJS) $(EX2_OBJS)

%.o:%$(EXT)
		$(CXX) $(CXXFLAGS) -c $<

dep:
		$(CXX) $(CXXFLAGS) -M $(CSRCS) $(EX1_SRCS) $(EX2_SRCS) > $(DEP)

clean:
		rm -rf $(EXEC1) $(EXEC2) $(EX1_OBJS) $(EX2_OBJS) $(COBJS) $(DEP)
