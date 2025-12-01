if [ ! -d "bin" ]; then
    mkdir bin
else
	rm bin/*
fi

g++ -g -O0 -I . -o bin/TA_marking_2B TA_marking_101258619_101166589.cpp TA_marking_2B_101258619_101166589.cpp -lpthread