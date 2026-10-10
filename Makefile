# ██╗   ██╗ █████╗ ██████╗ ██╗ █████╗ ██████╗ ██╗     ███████╗███████╗
# ██║   ██║██╔══██╗██╔══██╗██║██╔══██╗██╔══██╗██║     ██╔════╝██╔════╝
# ██║   ██║███████║██████╔╝██║███████║██████╔╝██║     █████╗  ███████╗
# ╚██╗ ██╔╝██╔══██║██╔══██╗██║██╔══██║██╔══██╗██║     ██╔══╝  ╚════██║
#  ╚████╔╝ ██║  ██║██║  ██║██║██║  ██║██████╔╝███████╗███████╗███████║
#   ╚═══╝  ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝╚═╝  ╚═╝╚═════╝ ╚══════╝╚══════╝╚══════╝

##@ -- Constants (Don't modify unless you exactly know what you are doing) --

## Name of this project
NAME    := qkiss
## Version, gotten from `./VERSION` file.
VERSION := $(shell cat VERSION)

TARBALL_BASENAME := $(NAME)-$(VERSION)
TARBALL_PATH := generated/$(TARBALL_BASENAME).tar.gz

## The root folder of all folders of configured CMake presets. This value is defined in `CMakePresets.json`
BUILD_ROOT_DIR := build

##@ -- General variables ---------------------------------------


## Use which CMake preset in `CMakePresets.json`. Default is `release`. (available: `debug`, `release`, `profile`, `minsize`)
CMAKE_PRESET := release


## The working directory for `rpmbuild`, for building RPM without `osc` (Default: `/tmp/rpmbuild/NAME/`)
RPMBUILD_DIR := /tmp/rpmbuild/$(NAME)

# The `.spec` file whose @VAR@ are already expanded by `cmake`
CONFIGURED_RPM_SPEC_FILE := generated/$(NAME).spec
# The `PKGBUILD` file whose @VAR@ are already expanded by `cmake`
CONFIGURED_PKGBUILD_FILE := generated/PKGBUILD

# The `debian/` folder, and all files whose @VAR@ are already expanded by `cmake`
CONFIGURED_DEBIAN_DIR := generated/debian/

##@ -- OBS (OpenBuildService) ------------------------------------

## A local package directory for `osc`. (ex: `~/osc/project:user/qkiss/`, created via `osc mkpac qkiss`)
OSC_PACKAGE_DIR ?=
# (expand ~ as $HOME)
override OSC_PACKAGE_DIR := $(subst ~,$(HOME),$(OSC_PACKAGE_DIR))

## OBS `osc build` target's repository name. `osc meta prj` to list. (ex: `openSUSE_Tumbleweed`, `Fedora_43`, `Debian_13`, `xUbuntu_25.04`, `Arch`)
OBS_REPO :=
## OBS `osc build` target's repository arch. `osc meta prj` to list.
OBS_ARCH :=

##@ -- Homepage (`homepage/`) ------------------------------------

## Where `homepage-build` writes the website, as GitHub Pages and GitLab Pages publish it. (Default: `generated/homepage`)
HOMEPAGE_OUT := generated/homepage
## Port `homepage-dev-server` serves the website on. (Default: `8000`)
HOMEPAGE_PORT := 8000
## A clone of the `qkiss-screenshots` repository, for the screenshots of the website. Empty: no screenshots. (ex: `~/proj/qkiss-screenshots`)
HOMEPAGE_SCREENSHOTS ?=
# (expand ~ as $HOME)
override HOMEPAGE_SCREENSHOTS := $(subst ~,$(HOME),$(HOMEPAGE_SCREENSHOTS))
# `homepage/build.py` wants either `--screenshots DIR` or `--no-screenshots`.
HOMEPAGE_SCREENSHOTS_OPTION = $(if $(HOMEPAGE_SCREENSHOTS),--screenshots "$(HOMEPAGE_SCREENSHOTS)",--no-screenshots)

# ██████╗ ███████╗███████╗██╗███╗   ██╗███████╗
# ██╔══██╗██╔════╝██╔════╝██║████╗  ██║██╔════╝
# ██║  ██║█████╗  █████╗  ██║██╔██╗ ██║█████╗
# ██║  ██║██╔══╝  ██╔══╝  ██║██║╚██╗██║██╔══╝
# ██████╔╝███████╗██║     ██║██║ ╚████║███████╗
# ╚═════╝ ╚══════╝╚═╝     ╚═╝╚═╝  ╚═══╝╚══════╝

define print_title
printf "\n"; \
printf "\033[36m=============================================================\033[0m\n"; \
printf "\033[36m %s \033[0m\n" $(1); \
printf "\033[36m=============================================================\033[0m\n" ;
endef

define __DOC_QUICK_START__
# Links
- Github: https://github.com/kuanyui/qkiss

# About this Makefile
- Use `make` or `make help` to show this help info.
- To set variables, use `make VAR1=foo VAR2=bar target`


endef
export __DOC_QUICK_START__


define __DOC_OSC_BUILD__
========================================================
# Then run:
osc build --clean --vm-type=kvm --vm-memory 8192 Debian_12 x86_64

# If failed, build witth --shell without --clean:
stty intr `Ctrl+v Ctrl+] Enter`
osc build --shell --vm-type=kvm --vm-memory 8192 Debian_12 x86_64

# With `--shell`, `/home/abuild/` should be empty (because it's before extracting tarball...):
ls /usr/src/packages/SOURCES/
dpkg-source -x /usr/src/packages/SOURCES/*.dsc
cd $(TARBALL_BASENAME)
cd /home/abuild && dpkg-source -x /.build-srcdir/*.dsc && cd $(TARBALL_BASENAME)/ && mkdir -p build && cd build && cmake ..
endef
export __DOC_OSC_BUILD__


.PHONY: help
help:    ## Show this self-documented help message.
	@# Some operating system / Linux distro may use `mawk` (e.g. Ubuntu), so prefer more portable `perl` over `awk`.
	@export S=`basename $${SHELL}`; if [ $${S} != "zsh" ] && [ $${S} != "fish" ]; then echo "(Tip: You are using \`$${S}\`, but you can consider to use \`zsh\` or \`fish\` because they support tab compoletions for Makefile targets & variables.)"; fi
	@if command -v perl >/dev/null 2>&1; then \
		$(call print_title,"Quick Start") \
	  	printf "$$__DOC_QUICK_START__" | perl -pe 's/`([^`]+)`/\033[33m`\1`\033[0m/g; s/^(#+ .+)/\033[32m\1\033[0m/g; s/(https?:\/\/[a-zA-Z0-9.\/_#?-]+)/\033[34m\1\033[0m/g'; \
		$(call print_title,"Variables") \
		perl -ne 'if (/^## /) { $$desc = substr($$_, 3); $$next = <>; if ($$next =~ /^([A-Za-z0-9_]+)\s*[:?+]?=\s*(.*)$$/) { $$target=$$1; $$desc=~s/(`[^`]*?`)/\033[33m\1\033[0m/g; printf "\033[35m%-30s\033[0m %s", $$target, $$desc; } } elsif (/^[ \t]*##@ *(.*)/) { print "\033[34m$$1\033[0m\n"; }' $(MAKEFILE_LIST); \
		$(call print_title,"Targets") \
		perl -ne 'if (/^([a-zA-Z0-9_-]+):.*?## (.*)$$/) { $$target = $$1; $$desc = $$2; if ($$desc =~ /DEPRECATED/) { printf "\033[2m%-30s %s\033[0m\n", $$target, $$desc; } else { $$desc=~s/(`[^`]*?`)/\033[33m\1\033[0m/g; printf "\033[32m%-30s\033[0m %s\n", $$target, $$desc; } } elsif (/^[ \t]*### *(.*)/) { print "\033[34m$$1\033[0m\n"; }' $(MAKEFILE_LIST); \
	else \
		$(call print_title,"Quick Start") \
		printf "$$__DOC_QUICK_START__" | gawk '{gsub(/`[^`]+`/, "\033[33m&\033[0m"); gsub(/^#+ .+/, "\033[32m&\033[0m"); gsub(/https?:\/\/[a-zA-Z0-9.\/_#?-]+/, "\033[34m&\033[0m"); print}' #(MAKEFILE_LIST)\
		$(call print_title,"Variables") \
		gawk '/^## /{desc=substr($$0, 4); gsub(/`[^`]*`/, "\033[33m&\033[0m", desc); getline; if ($$1 ~ /^[A-Za-z0-9_]+$$/ && match($$0, /^[^:?+]*[:?+]?=/, m)) printf "\033[35m%-30s\033[0m %s\n", $$1, desc}' $(MAKEFILE_LIST) ; \
		$(call print_title,"Targets") \
		gawk 'match($$0, /^([a-zA-Z0-9_-]+):.*?## (.*)$$/, m) { target=m[1]; desc=m[2]; if (desc ~ /DEPRECATED/) { printf "\033[2m%-30s %s\033[0m\n", target, desc } else { gsub(/`[^`]*`/, "\033[33m&\033[0m", desc); printf "\033[32m%-30s\033[0m %s\n", target, desc } } match($$0, /^[ \\t]*### *(.*)/, m){printf "\033[34m%s\033[0m\n", m[1]}' $(MAKEFILE_LIST); \
	fi



.PHONY: clean
clean:    ## Clean all built files (CMake & built icons).
	rm -rf resources/tango-remixer/dist/*
	rm -rf "$(BUILD_ROOT_DIR)"/*
	rm -rf generated/*
	# Because I overwrite rpmbuild's _topdir to /tmp (tmpfs)
	rm -rf "$(RPMBUILD_DIR)"/*

.PHONY: clean-all
clean-all: clean   ## Clean all (built files & CMake cache)
	rm -f CMakeLists.txt.user
	rm -f .qtcreator/CMakeLists.txt.user

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
uninstall-git-hooks:     ## Uninstall git-hooks and commit-template.
	rm .git/hooks/commit-msg
	rm .git/hooks/pre-commit
	git config --unset commit.template

.PHONY: add-license-header
add-license-header:    ## Add license header to all .cpp / .h files
	./scripts/add_license_header.py

###
### -- Homepage (`homepage/`) -----------------------------------

# The website as plain files, as GitHub Pages and GitLab Pages publish it.
.PHONY: homepage-build
homepage-build:    ## Build the website into `HOMEPAGE_OUT` (with the screenshots of `HOMEPAGE_SCREENSHOTS`, if set).
	rm -rf "$(HOMEPAGE_OUT)"
	python3 homepage/build.py "$(HOMEPAGE_OUT)" $(HOMEPAGE_SCREENSHOTS_OPTION)

# The same, built into a temporary folder (not `HOMEPAGE_OUT`) and served on every interface, so the host of a VM
# can open it too. The URLs to open are printed, this machine's LAN addresses among them. A change to
# homepage/src/, the app icons or the screenshots rebuilds the site while it is served (reload the page).
.PHONY: homepage-dev-server
homepage-dev-server:    ## Build the website, serve it on port `HOMEPAGE_PORT` on every interface (the URLs to open are printed, LAN addresses included) and rebuild it when its files change. Ctrl-C stops it.
	python3 homepage/build.py --serve $(HOMEPAGE_PORT) $(HOMEPAGE_SCREENSHOTS_OPTION)


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
build-rpm-local-rpmbuild: $(TARBALL_PATH) _configure  ## Local build rpm with `rpmbuild` without `osc`
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
	cp -f "${CONFIGURED_PKGBUILD_FILE}" "$(OSC_PACKAGE_DIR)/"

.PHONY: osc-create-debian-dsc
osc-create-debian-dsc: _check_osc_var osc-cp
	cd "$(OSC_PACKAGE_DIR)/"; \
    tar xvf "$(TARBALL_BASENAME).tar.gz"; \
	dpkg-source -b "$(TARBALL_BASENAME)"; \
	rm -rf "$(TARBALL_BASENAME)"

.PHONY: osc-build-debian
osc-build-debian: _check_osc_var osc-create-debian-dsc
	cd "$(OSC_PACKAGE_DIR)/"; \
	osc build --vm-type=kvm --vm-memory=16000M $(OBS_REPO) $(OBS_ARCH)

.PHONY: osc-fix-broken-vm-env
osc-fix-broken-vm-env: _check_osc_var
	cd "$(OSC_PACKAGE_DIR)/"; rm -f .osc/_buildinfo* .osc/_buildconfig*
	# Delete QEMU/KVM img
	rm -rf /var/tmp/build-root-$(USER)/$(OBS_REPO)-$(OBS_ARCH)
