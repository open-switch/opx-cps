import sys
import os
from subprocess import call
import subprocess
import ar_yin_reader

if __name__ == '__main__':
	cfg_file = sys.argv[1]
	tmp_path = sys.argv[2]
	out_path = sys.argv[3]
	
	relative_path = os.path.dirname(cfg_file)
	
	f = open (cfg_file)
	for l in f:
		model_file = l
		yang_model = model_file.split(".yang")[0];
		yin_file = tmp_path+"/"+yang_model+".yin"
		history_file = relative_path+"/"+yang_model + ".hist"
		model_file = relative_path+"/"+model_file
		p = Popen (["pyang","-f","yin"],stdout=PIPE)
		of = open(yin_file,"w");
		for out in p:
			of.write(out)
		of.close()
		
		old_of = sys.stdout
		sys.stdout = open(out_path+"/"+yang_model+".h")
		yin_parser.process(yin_file,history_file)
		sys.stdout.close()
		sys.stdout = old_of
		