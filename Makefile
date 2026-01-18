.PHONY: help
help:
	@echo "Zynq CNN Accelerator - Quick Reference"
	@echo ""
	@echo "Common Commands:"
	@echo "  make setup          - Run environment setup script"
	@echo "  make quantize       - Quantize CNN model"
	@echo "  make hls            - Build HLS IP cores"
	@echo "  make software       - Build embedded software"
	@echo "  make deploy         - Deploy to Zynq board (set TARGET_IP)"
	@echo "  make benchmark      - Run performance benchmarks"
	@echo "  make clean-all      - Clean all build artifacts"
	@echo ""

.PHONY: setup
setup:
	@bash scripts/setup_env.sh

.PHONY: quantize
quantize:
	@cd models && python3 quantize_model.py --model imagenet --output quantized/

.PHONY: hls
hls:
	@cd scripts && vitis_hls -f build_hls.tcl

.PHONY: software
software:
	@$(MAKE) -C software all

.PHONY: deploy
deploy:
	@$(MAKE) -C software deploy TARGET_IP=$(TARGET_IP)

.PHONY: benchmark
benchmark:
	@cd scripts && python3 run_benchmarks.py --iterations 100

.PHONY: clean-all
clean-all:
	@$(MAKE) -C software clean
	@rm -rf hardware/hls/*/cnn_accelerator
	@echo "Cleaned all build artifacts"
