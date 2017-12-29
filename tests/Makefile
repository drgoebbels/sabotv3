
.PHONY: all
all: general-test-gen.out pool-test-gen.out

.PHONY: clean
clean:
	@echo "---calling clean---"
	@rm -f *-test-gen.c > /dev/null
	@rm -f *-test-gen.out > /dev/null

.PHONY: general
general: general-test-gen.out

general-test-gen.out:
	@echo "---calling general-test-gen.out---"
	./general-test.pl > general-test-gen.c
	cc ../general.c general-test-gen.c -o general-test-gen.out

.PHONY: pool 
pool: pool-test-gen.out

pool-test-gen.out:
	@echo "---calling pool-test-gen.out---"
	cc -lpthread ../pool.c ../log.c ../general.c pool-test.c -o pool-test-gen.out


.PHONY: check
check: general-test-gen.out pool-test-gen.out
	@echo "---calling check---"
	@echo "---Testing Module 'general'---"
	./general-test-gen.out


