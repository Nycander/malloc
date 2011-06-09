
TIMES = 5

algorithms = [ "System", "First Fit", "Best Fit", "Worst Fit", "Quick Fit" ]

# Compile everything!
system("make -B")

algorithms.each_index do | strategy |
 	print algorithms[strategy], "\t"

 	name = algorithms[strategy]
 	cmd = name.downcase.gsub(/ /, '');

 	# We need an average of a couple of rounds, since we're dealing with time
 	
 	0.upto(TIMES) do | i |
 		out = `/usr/bin/time ./time_#{cmd} 2>&1`
 		print out.match(/^\s*(\d+.\d+).*?/).to_s.to_f, "\t"
 	end

 	print "\n"

 end