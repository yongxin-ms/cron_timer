cc = /usr/bin/g++
prom = ./cron_timer_test
deps = $(shell find ./ -name "*.h")
src = $(shell find ./ -name "*.cpp")
obj = $(src:%.cpp=%.o)

CXXFLAGS += -std=c++11 -Wl,--no-as-needed -Wall

#链接选项及搜索目录
LIB_PATH = /usr/lib
LIB_LOCAL_PATH = /usr/local/lib
LDFLAGS = -L$(LIB_PATH) -L$(LIB_LOCAL_PATH)
LDLIBS = -ldl -lpthread

$(prom): $(obj)
	$(cc) $(LDFLAGS) -o $(prom) $(obj) $(LDLIBS)

%.o: %.cpp $(deps)
	$(cc) -c $< -o $@ $(CXXFLAGS)

clean:
	rm -rf $(obj) $(prom)


