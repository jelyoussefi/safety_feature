#----------------------------------------------------------------------------------------------------------------------
# Flags
#----------------------------------------------------------------------------------------------------------------------
SHELL:=/bin/bash

CC=gcc
#----------------------------------------------------------------------------------------------------------------------
# Targets
#----------------------------------------------------------------------------------------------------------------------
default: run 
.PHONY:  

install_prerequisites:
	@make -C ./deps/ install


%.o: %.cpp
	@$(CC) -O2 -c -o $@    $<
	
safety_feature: safety_feature.o  main.o 
	@$(call msg,Building the safety featue applicatio  ...)
	$(CC) -O2  $^ -lstdc++  -o $@

run: safety_feature
	@$(call msg,Running the safety feaure ...)
	@./safety_feature


clean:
	@rm -rf  *.o ./safety_feature



#----------------------------------------------------------------------------------------------------------------------
# helper functions
#----------------------------------------------------------------------------------------------------------------------
define msg
	tput setaf 2 && \
	for i in $(shell seq 1 120 ); do echo -n "-"; done; echo  "" && \
	echo "         "$1 && \
	for i in $(shell seq 1 120 ); do echo -n "-"; done; echo "" && \
	tput sgr0
endef

