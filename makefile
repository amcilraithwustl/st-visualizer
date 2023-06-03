


frontend: backend
	cd st-visualizer-electron
	npm install
	npm run make

backend: 
	$(MAKE) -C st-visualizer
	cp st-visualizer/bin/st-visualizer st-visualizer-electron/imports/static/st-visualizer

clean:
	$(MAKE) -C st-visualizer clean
	rm -rf st-visualizer-electron/out
	rm -rf st-visualizer-electron/imports/static/st-visualizer