
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

.PHONY: test-pool
test-pool: pool-test-gen.out
	@echo "---testing thread pool---"
	@echo "...test size 20, pool size 1"
	@./pool-test-gen.out 20 1 > /dev/null
	@echo "...test size 30, pool size 2"
	@./pool-test-gen.out 30 2 > /dev/null
	@echo "...test size 100, pool size 8"
	@./pool-test-gen.out 100 8 > /dev/null
	@echo "...test size 500, pool size 32"
	@./pool-test-gen.out 500 32 > /dev/null
	@echo "...test size 1000, pool size 200"
	@./pool-test-gen.out 1000 200 > /dev/null

.PHONY: check
check: general-test-gen.out pool-test-gen.out
	@echo "---calling check---"
	@echo "---Testing Module 'general'---"
	./general-test-gen.out


