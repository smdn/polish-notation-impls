MYPY = mypy --config-file ../../../mypy.ini

all: type-check run

type-check:
	$(MYPY) polish.py

run:
	@if [ -z "${INPUT}" ]; then \
		./polish.py; \
	fi

	@if [ -n "${INPUT}" ]; then \
		echo ${INPUT} | ./polish.py; \
	fi

test:
	../../../tests/impls/run-tests.ps1 --target-impl python
