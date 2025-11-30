.PHONY: help clear install-git-hooks add-license-header

## Name of this project
NAME    := qkiss
TARBALL := $(NAME).tar.gz
RPMBUILD_DIR := $(HOME)/rpmbuild
SPEC_FILE := packaging/$(NAME).spec

help:     ## Show this self-documented help message.
	@# Some operating system / Linux distro may use `mawk` (e.g. Ubuntu), so prefer more portable `perl` over `awk`.
	@export S=`basename $${SHELL}`; if [ $${S} != "zsh" ] && [ $${S} != "fish" ]; then echo "(Tip: You are using \`$${S}\`, but you can consider to use \`zsh\` or \`fish\` because they support tab compoletions for Makefile targets & variables.)"; fi

	@if command -v perl >/dev/null 2>&1; then \
		printf "\033[36m=============================================================\033[0m\n" ; \
		printf "\033[36m Variables                                                   \033[0m\n" ; \
		printf "\033[36m=============================================================\033[0m\n" ; \
		perl -ne 'if (/^## /) { $$comment = $$_; $$next = <>; if ($$next =~ /^([A-Za-z0-9_]+)\s*[:?+]?=\s*(.*)$$/) { printf "\033[35m%-30s\033[0m %s", $$1, substr($$comment, 3); } }' $(MAKEFILE_LIST); \
		printf "\n"; \
		printf "\033[36m=============================================================\033[0m\n" ; \
		printf "\033[36m Targets                                                     \033[0m\n" ; \
		printf "\033[36m=============================================================\033[0m\n" ; \
		perl -ne 'if (/^([a-zA-Z0-9_-]+):.*?## (.*)$$/) { printf "\033[32m%-30s\033[0m %s\n", $$1, $$2; } elsif (/^[ \t]*### *(.*)/) { print "\033[34m$$1\033[0m\n"; }' $(MAKEFILE_LIST); \
	else \
		printf "\033[36m=============================================================\033[0m\n" ; \
		printf "\033[36m Variables                                                   \033[0m\n" ; \
		printf "\033[36m=============================================================\033[0m\n" ; \
		gawk '/^## /{comment=$$0; getline; if ($$1 ~ /^[A-Za-z0-9_]+$$/ && match($$0, /^[^:?+]*[:?+]?=/, m)) printf "\033[35m%-30s\033[0m %s\n", $$1, substr(comment, 4)}' $(MAKEFILE_LIST) ; \
		printf "\n"; \
		printf "\033[36m=============================================================\033[0m\n" ; \
		printf "\033[36m Targets                                                     \033[0m\n" ; \
		printf "\033[36m=============================================================\033[0m\n" ; \
		gawk 'match($$0, /^([a-zA-Z0-9_-]+):.*?## (.*)$$/, m){printf "\033[32m%-30s\033[0m %s\n", m[1], m[2]} match($$0, /^[ \\t]*### *(.*)/, m){printf "\033[34m%s\033[0m\n", m[1]}' $(MAKEFILE_LIST); \
	fi

clean:    ## Clean all built files (CMake & built icons).
	rm -rf resources/tango-icon-remixer/dist/*
	rm -rf build/*
	rm -f $(TARBALL)

install-git-hooks:     ## Install git-hooks and commit-template.
	cp scripts/git_hooks/commit-msg .git/hooks/commit-msg
	cp scripts/git_hooks/pre-commit .git/hooks/pre-commit
	chmod +x .git/hooks/commit-msg
	chmod +x .git/hooks/pre-commit
	git config commit.template scripts/git_hooks/commit-template.txt

uninstall-git-hooks:     ## Install git-hooks and commit-template.
	rm .git/hooks/commit-msg
	rm .git/hooks/pre-commit
	git config --unset commit.template

add-license-header:    ## Add license header to all .cpp / .h files
	./scripts/add_license_header.py

.PHONY: $(TARBALL)
$(TARBALL):  ## Create tarball in this repo root dir.
	tar --exclude-vcs --exclude=$(TARBALL) --exclude=build --exclude=samples --exclude=$(TARBALL) --transform='s,^\./,qkiss-master/,' -czf $@ .

build-rpm-local-rpmbuild: $(TARBALL)   ## Local build rpm with `rpmbuild` into `~/rpmbuild`
	rm -rf ~/rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}/*
	mkdir -p ~/rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
	cp -f $(TARBALL) $(RPMBUILD_DIR)/SOURCES/
	cp -f ${SPEC_FILE} $(RPMBUILD_DIR)/SPECS/
	rpmbuild -ba $(RPMBUILD_DIR)/SPECS/$(NAME).spec

