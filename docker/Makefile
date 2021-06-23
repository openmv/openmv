.PHONY: default
default: in-docker-build

.PHONY: build
build:                                   
	./scripts/build

./.dapper:
	@echo Downloading dapper
	@curl -sL https://releases.rancher.com/dapper/v0.5.0/dapper-$$(uname -s)-$$(uname -m) > .dapper.tmp
	@@chmod +x .dapper.tmp
	@./.dapper.tmp -v
	@mv .dapper.tmp .dapper

in-docker-%: .dapper
	./.dapper -C ../ -f docker/Dockerfile --target dapper make $*
