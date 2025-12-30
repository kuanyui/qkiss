## Name of this project
NAME    := qkiss
## Version, gotten from `./VERSION` file.
VERSION := $(shell cat VERSION)

TARBALL_BASENAME := $(NAME)-$(VERSION)
TARBALL_PATH := generated/$(TARBALL_BASENAME).tar.gz

## The working directory for `rpmbuild` (Default: `/tmp/rpmbuild/NAME/`)
RPMBUILD_DIR := /tmp/rpmbuild/$(NAME)

## The root folder of all folders of configured CMake presets. This value is defined in `CMakePresets.json`
BUILD_ROOT_DIR := build

## Use which CMake preset in `CMakePresets.json`. Default is `release`. (available: `debug`, `release`, `profile`, `minsize`)
CMAKE_PRESET := release

## The `.spec` file whose @VAR@ are already expanded by `cmake`
CONFIGURED_RPM_SPEC_FILE := generated/$(NAME).spec

## The `debian/` folder, and all files whose @VAR@ are already expanded by `cmake`
CONFIGURED_DEBIAN_DIR := generated/debian/

## For OpenBuildService, a package directory for `osc`. (ex: `~/osc/project:user/package/`)
OSC_PACKAGE_DIR ?=
# (expand ~ as $HOME)
override OSC_PACKAGE_DIR := $(subst ~,$(HOME),$(OSC_PACKAGE_DIR))

.PHONY: help
help:    ## Show this self-documented help message.
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

.PHONY: clean
clean:    ## Clean all built files (CMake & built icons).
	rm -rf resources/tango-icon-remixer/dist/*
	rm -rf "$(BUILD_ROOT_DIR)"/*
	rm -rf generated/*

.PHONY: clean-cmake-cache
clean-cmake-cache: clean   ## Clean all (built files & CMake cache)
	rm -f CMakeLists.txt.user

# ██████╗ ███████╗██╗   ██╗    ████████╗ ██████╗  ██████╗ ██╗     ███████╗
# ██╔══██╗██╔════╝██║   ██║    ╚══██╔══╝██╔═══██╗██╔═══██╗██║     ██╔════╝
# ██║  ██║█████╗  ██║   ██║       ██║   ██║   ██║██║   ██║██║     ███████╗
# ██║  ██║██╔══╝  ╚██╗ ██╔╝       ██║   ██║   ██║██║   ██║██║     ╚════██║
# ██████╔╝███████╗ ╚████╔╝        ██║   ╚██████╔╝╚██████╔╝███████╗███████║
# ╚═════╝ ╚══════╝  ╚═══╝         ╚═╝    ╚═════╝  ╚═════╝ ╚══════╝╚══════╝
###
### -- Development Tools ----------------------------------------

.PHONY: install-git-hooks
install-git-hooks:     ## Install git-hooks and commit-template.
	cp scripts/git_hooks/commit-msg .git/hooks/commit-msg
	cp scripts/git_hooks/pre-commit .git/hooks/pre-commit
	chmod +x .git/hooks/commit-msg
	chmod +x .git/hooks/pre-commit
	git config commit.template scripts/git_hooks/commit-template.txt

.PHONY: uninstall-git-hooks
uninstall-git-hooks:     ## Install git-hooks and commit-template.
	rm .git/hooks/commit-msg
	rm .git/hooks/pre-commit
	git config --unset commit.template

.PHONY: add-license-header
add-license-header:    ## Add license header to all .cpp / .h files
	./scripts/add_license_header.py


# ████████╗ █████╗ ██████╗ ██████╗  █████╗ ██╗     ██╗
# ╚══██╔══╝██╔══██╗██╔══██╗██╔══██╗██╔══██╗██║     ██║
#    ██║   ███████║██████╔╝██████╔╝███████║██║     ██║
#    ██║   ██╔══██║██╔══██╗██╔══██╗██╔══██║██║     ██║
#    ██║   ██║  ██║██║  ██║██████╔╝██║  ██║███████╗███████╗
#    ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═╝╚═════╝ ╚═╝  ╚═╝╚══════╝╚══════╝
###
### -- Tarball & Packaging --------------------------------------

.PHONY: _configure
_configure:    # (internal, for tarball) Run `cmake -B` to configure files
	cmake -B $(BUILD_ROOT_DIR) -S .

.PHONY: $(CONFIGURED_RPM_SPEC_FILE)
$(CONFIGURED_RPM_SPEC_FILE): _configure   # (Internal, for tarball) Run cmake to configure RPM `.spec.in`

.PHONY: gitversion.cmake
gitversion.cmake:    # (For tarball) Because tarball doesn't contain .git history so store commit hash into .cmake file.
	echo 'set(GIT_COMMIT_HASH "$(shell git rev-parse --short HEAD)")' > $@
	echo 'set(GIT_COMMIT_TIME_UNIX_SEC "$(shell git show --no-patch --format=%ct HEAD)")' >> $@

.PHONY: $(TARBALL_PATH)
$(TARBALL_PATH): gitversion.cmake _configure  # (Internal, for tarball)
	cp -r generated/debian ./
	tar --exclude-vcs \
	    --exclude=$(TARBALL_PATH) \
		--exclude=$(BUILD_ROOT_DIR) \
		--exclude=CMakeLists.txt.user \
		--exclude=generated \
		--exclude=dist \
		--exclude=__pycache__ \
		--exclude=humanity_0.4.6 \
		--exclude=tango-icon-theme-0.8.90 \
		--exclude=samples \
		--exclude=$(TARBALL_PATH) \
		--transform='s|^[.]/|$(TARBALL_BASENAME)/|' -czf $@ .
	rm gitversion.cmake
	rm -r debian/

.PHONY: release-tarball
release-tarball: $(TARBALL_PATH)    ## Archive the codebase along with `gitversion.cmake` as tarball

.PHONY: build-rpm-local-rpmbuild
build-rpm-local-rpmbuild: $(TARBALL_PATH) $(CONFIGURED_RPM_SPEC_FILE) _configure  ## Local build rpm with `rpmbuild`
	rm -rf "$(RPMBUILD_DIR)"/*
	mkdir -p $(RPMBUILD_DIR)/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
	cp -f $(TARBALL_PATH) $(RPMBUILD_DIR)/SOURCES/
	cp -f ${CONFIGURED_RPM_SPEC_FILE} $(RPMBUILD_DIR)/SPECS/

	rpmbuild -ba --define "_topdir $(RPMBUILD_DIR)" "$(RPMBUILD_DIR)/SPECS/$(NAME).spec"


_check_osc_var:
ifeq ($(strip $(OSC_PACKAGE_DIR)),)
	$(error Makefile variable OSC_PACKAGE_DIR is not defined.)
endif
ifeq ($(wildcard $(OSC_PACKAGE_DIR)/.),)
	$(error Directory '$(OSC_PACKAGE_DIR)' does not exist)
endif

.PHONY: osc-cp
osc-cp: _check_osc_var $(TARBALL_PATH)    ## Copy necessary files to osc package folder
	cp -f "$(TARBALL_PATH)" "$(OSC_PACKAGE_DIR)/"
	cp -f "${CONFIGURED_RPM_SPEC_FILE}" "$(OSC_PACKAGE_DIR)/"

.PHONY: osc-create-debian-dsc
osc-create-debian-dsc: _check_osc_var osc-cp
	cd "$(OSC_PACKAGE_DIR)/"; \
    tar xvf "$(TARBALL_BASENAME).tar.gz"; \
	dpkg-source -b "$(TARBALL_BASENAME)"; \
	rm -rf "$(TARBALL_BASENAME)"

.PHONY: osc-build-debian
osc-build-debian: _check_osc_var osc-create-debian-dsc
	cd "$(OSC_PACKAGE_DIR)/"; \
	osc build --vm-type=kvm --vm-memory=12000M Debian_12 x86_64
