from a51 import *

class table_gen:
    def __init__(self, end_point_bit, max_chain_len, max_number_chains):
        self.end_point_mask = (1 << end_point_bit) - 1
        self.max_number_chains = max_number_chains
        self.max_chain_len = max_chain_len
        self.reduction_function = 0

    def gen(self, reduction_function):
        self.reduction_function = reduction_function
        for chain_number in xrange(0,self.max_number_chains):
            start_point = self._gen_start_point(chain_number)
            (max_chain_len_exceed, end_point, chain_length) = self._gen_chain(start_point)

            if max_chain_len_exceed == False:    #max_chain_len_exceed is looping
                print (start_point, end_point, chain_length)

    def _gen_start_point(self, chain_number):
        start_point = ((chain_number+1) << (18+22))+((chain_number+1) << (18))+(chain_number+1)
        return start_point  #TODO: find better method to generate start_points of chains

    def _gen_chain(self, start_point): #a5_until_endpoint
        alg = a51()
        state = start_point
        alg.set_state(state)

        print "Generating a chain"

        for chain_links in xrange(0, max_chain_len):
            output = alg.gen_block(64)
            print "State:%x -> Output:%x" % (state, output)
            if self._is_endpoint(output):
                return (False, output, chain_links)

            state = self._reduction(output)
            alg.set_state(state)

        return (True, None, None)

    def _is_endpoint(self, output):
        if (output & self.end_point_mask) == 0:
            return True
        else:
            return False

    def _reduction(self, output):
        return output ^ self.reduction_function

def gen_tables(max_tables, end_point_bit, max_chain_len, max_number_chains):
    generator = table_gen(end_point_bit, max_chain_len, max_number_chains)
    for table_nr in xrange(1,max_tables):
        reduction_function = table_nr
        generator.gen(reduction_function)

if __name__ == '__main__':
    max_tables = 156
    max_number_chains = 2000
    end_point_bit = 27
    max_chain_len = 2000

    gen_tables(max_tables, end_point_bit, max_chain_len, max_number_chains)
