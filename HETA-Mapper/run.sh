# SPDLOG_LEVEL=trace, debug, info, warn, err, critical, off
./build/cgra-compiler SPDLOG_LEVEL=off \
	-c true -m true -o false -u true -h false -t 30000 -i 3000 \
	-p "../cgra-mg/src/main/resources/operations.json" \
	-a "../cgra-mg/src/main/resources/cgra_adg.json" \
	-d "../benchmark/arf/arf.json ../benchmark/centro-fir/centro-fir.json ../benchmark/ewf/ewf.json \
	 	../benchmark/cosine2/cosine2.json ../benchmark/fft/fft.json ../benchmark/fir/fir.json\
	 	../benchmark/fir1/fir1.json ../benchmark/resnet2/resnet2.json ../benchmark/stencil3d/stencil3d.json"
	#-d "../bechmarks/CGRA_FRAM/test/fft/dfg_modified.json"
	# -d "../bechmarks/LSU/test/stencil3d/stencil3d.json"
	#-d "../bechmarks/CGRA_FRAM/test/conv3/conv3.json"
  #-d "../bechmarks/test/arf/arf.json ../bechmarks/test/test_imul/imul.json ../bechmarks/test/test_LSU_1/LSU.json ../bechmarks/test/cosine1/cosine1.json"
	#-d "../bechmarks/test/test_imul/imul.json"
	#-d "../bechmarks/test/arf/arf.json ../bechmarks/test/centro-fir/centro-fir.json ../bechmarks/test/cosine1/cosine1.json ../bechmarks/test/ewf/ewf.json ../bechmarks/test/fir1/fir1.json ../bechmarks/test/resnet1/resnet1.json"
	# -d "../bechmarks/test/fft/fft.json"
	# -d "../bechmarks/test/cosine2/cosine2.json ../bechmarks/test/fir/fir.json ../bechmarks/test/md/md.json ../bechmarks/test/resnet2/resnet2.jso
