.PHONY: help clear install-git-hooks add-license-header

help:     ## Show this self-documented help message.
	@# Some operating system / Linux distro may use `mawk` (e.g. Ubuntu), so prefer more portable `perl` over `awk`.
	@export S=`basename $${SHELL}`; if [ $${S} != "zsh" ] && [ $${S} != "fish" ]; then echo "(Tip: You are using \`$${S}\`, but you can consider to use \`zsh\` or \`fish\` because they support tab compoletions for Makefile targets & variables.)"; fi
	@if command -v perl >/dev/null 2>&1; then \
		echo ------------------------ ; \
		echo Variables ; \
		echo ------------------------ ; \
		perl -ne 'if (/^## /) { $$comment = $$_; $$next = <>; if ($$next =~ /^([A-Za-z0-9_]+)\s*[:?+]?=\s*(.*)$$/) { printf "\033[35m%-30s\033[0m %s", $$1, substr($$comment, 3); } }' $(MAKEFILE_LIST); \
		echo ------------------------ ; \
		echo Targets ; \
		echo ------------------------ ; \
		perl -ne 'if (/^([a-zA-Z0-9_-]+):.*?## (.*)$$/) { printf "\033[36m%-30s\033[0m %s\n", $$1, $$2; } elsif (/^[ \t]*### *(.*)/) { print "$$1\n"; }' $(MAKEFILE_LIST); \
	else \
		echo ------------------------ ; \
		echo Variables ; \
		echo ------------------------ ; \
		gawk '/^## /{comment=$$0; getline; if ($$1 ~ /^[A-Za-z0-9_]+$$/ && match($$0, /^[^:?+]*[:?+]?=/, m)) printf "\033[35m%-30s\033[0m %s\n", $$1, substr(comment, 4)}' $(MAKEFILE_LIST) ; \
		echo ------------------------ ; \
		echo Targets ; \
		echo ------------------------ ; \
		gawk 'match($$0, /^([a-zA-Z0-9_-]+):.*?## (.*)$$/, m){printf "\033[36m%-30s\033[0m %s\n", m[1], m[2]} match($$0, /^[ \\t]*### *(.*)/, m){printf "%s\n", m[1]}' $(MAKEFILE_LIST); \
	fi

clear:
	rm -rf resources/tango-icon-remixer/dist/*
	rm -rf build/*

install-git-hooks:     ## install git-hooks and commit-template
	cp scripts/git_hooks/commit-msg .git/hooks/commit-msg
	cp scripts/git_hooks/pre-commit .git/hooks/pre-commit
	chmod +x .git/hooks/commit-msg
	chmod +x .git/hooks/pre-commit
	git config commit.template scripts/git_hooks/commit-template.txt

uninstall-git-hooks:     ## install git-hooks and commit-template
	rm .git/hooks/commit-msg
	rm .git/hooks/pre-commit
	git config --unset commit.template

add-license-header:
	./scripts/add_license_header.py
