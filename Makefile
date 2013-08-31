CXXFLAGS	= -O2 -g -Wall -fmessage-length=0 -std=c++0x

OBJS	= src/data/bdd_package.o src/data/bdd.o src/data/canonical.o src/data/minterm.o src/data/path_space.o src/data/path.o src/data/petri.o src/data/triple.o src/data/variable_space.o src/data/variable.o \
		  src/syntax/assignment.o src/syntax/composition.o src/syntax/condition.o src/syntax/control.o src/syntax/debug.o src/syntax/guard.o src/syntax/instruction.o src/syntax/loop.o src/syntax/parallel.o src/syntax/rule_space.o src/syntax/rule.o src/syntax/sequential.o src/syntax/skip.o \
		  src/type/channel.o src/type/keyword.o src/type/operator.o src/type/process.o src/type/record.o \
		  src/chp.o src/common.o src/flag_space.o src/program.o src/utility.o \

LIBS	=

TARGET	= haystack.exe

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
