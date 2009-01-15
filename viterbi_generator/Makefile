Lh=4
CXX=g++
CFLAGS=-O

viterbi_detector.c:  ./utils/viterbi_generator.m ./utils/lower_utils/equations_gen.m make_function.m
	@./make_function.m  $(Lh)

test:  tests/test_gen.m tests/test_check.m ./utils/viterbi_generator.m ./utils/lower_utils/equations_gen.m 
	@./tests/$@_gen.m $(Lh)
	@$(CXX) $(LDFLAGS) $(CFLAGS) $@.cpp -o $@ 
	@./$@ > $@_result
#	@less $@_result
#	@rm $@.c
	@rm $@ 
	@./tests/$@_check.m $(Lh)
	@rm $@_result

    
clean:
	rm -f *.o
	rm -f *.c
	rm *~
