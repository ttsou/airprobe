def gen_a51():
    first_100_steps= []
    first_100_steps.append("   wire [StateWidth:1] step1_out;")
    first_100_steps.append("   one_step step1(.clk(clk), .enable(enable), .in_state(in_state), .out_state(step1_out));\n")

    for i in xrange(2,101):
         first_100_steps.append("   wire [StateWidth:1] step%d_out;"%(i))
         first_100_steps.append("   one_step step%d(.clk(clk), .enable(enable), .in_state(step%d_out), .out_state(step%d_out));\n"%(i,(i-1),i))
         

    for line in first_100_steps:
        print line


gen_a51()
        
        
