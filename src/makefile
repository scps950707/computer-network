CSRCS = tool.cpp shake.cpp slow.cpp nat.cpp delay.cpp conavoid.cpp fastretrans.cpp fastrecovery.cpp sack.cpp
EX1_SRCS = server.cpp
EX2_SRCS = client.cpp
EX3_SRCS = node.cpp
EXT = .cpp
COBJS = $(CSRCS:$(EXT)=.o)
EX1_OBJS = $(EX1_SRCS:$(EXT)=.o)
EX2_OBJS = $(EX2_SRCS:$(EXT)=.o)
EX3_OBJS = $(EX3_SRCS:$(EXT)=.o)
EXEC1 = server
EXEC2 = client
EXEC3 = node
CXX = g++
CXXFLAGS = -Wall -std=c++11
DEP = dep

all: $(EXEC1) $(EXEC2) $(EXEC3)

ifdef debug
CXXFLAGS += -D__DEBUG__
endif

ifdef seq
CXXFLAGS += -D__SEQSTATIC__
endif

ifdef rand
CXXFLAGS += -D__USESRAND__
endif

ifdef slow
CXXFLAGS += -D__SLOW__
endif

ifdef delay
CXXFLAGS += -D__DELAY__
endif

ifdef conavoid
CXXFLAGS += -D__CONAVOID__
endif

ifdef fastre
CXXFLAGS += -D__FASTRE__
endif

ifdef fastcov
CXXFLAGS += -D__FASTCOV__
endif

ifdef sack
CXXFLAGS += -D__SACK__
endif

$(EXEC1): $(COBJS) $(EX1_OBJS)
		$(CXX) $(CXXFLAGS) -o $@ $(COBJS) $(EX1_OBJS)

$(EXEC2): $(COBJS) $(EX2_OBJS)
		$(CXX) $(CXXFLAGS) -o $@ $(COBJS) $(EX2_OBJS)

$(EXEC3): $(COBJS) $(EX3_OBJS)
		$(CXX) $(CXXFLAGS) -o $@ $(COBJS) $(EX3_OBJS)

%.o:%$(EXT)
		$(CXX) $(CXXFLAGS) -c $<

dep:
		$(CXX) $(CXXFLAGS) -M $(CSRCS) $(EX1_SRCS) $(EX2_SRCS) > $(DEP)

clean:
		rm -rf $(EXEC1) $(EXEC2) $(EXEC3) $(EX1_OBJS) $(EX2_OBJS) $(EX3_OBJS) $(COBJS) $(DEP) output
