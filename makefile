
rootdir = $(realpath .)/

frontend: backend
	$(MAKE) -C st-visualizer-electron

backend: 
	$(MAKE) -C st-visualizer
	cp $(rootdir)st-visualizer/bin/st-visualizer $(rootdir)st-visualizer-electron/imports/static/st-visualizer

clean:
	$(MAKE) -C st-visualizer clean
	rm -rf st-visualizer-electron/out
	rm -rf st-visualizer-electron/imports/static/st-visualizer