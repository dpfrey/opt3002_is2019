is2019_dev_day.wp77xx.update:
	mksys -t wp77xx is2019_dev_day.sdef

.PHONY: install
install: is2019_dev_day.wp77xx.update
	update is2019_dev_day.wp77xx.update 192.168.2.2

.PHONY: clean
clean:
	rm -rf _build_is2019_dev_day
	rm -f is2019_dev_day.wp77xx.update

