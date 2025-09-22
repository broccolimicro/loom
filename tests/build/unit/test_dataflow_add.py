import cocotb
from cocotb.triggers import RisingEdge
from cocotb.clock import Clock

import numpy as np
import matplotlib.pyplot as plt

import pytest
import random

from puffs import test
from puffs import fixed
from puffs import channel

from models import dataflow

@cocotb.test()
async def cocotb_dataflow_add(dut):
	params = test.Params()
	log = test.Logger()

	# Seed logging
	seed = random.randint(-1_000_000_000, 1_000_000_000)
	log.info(f"seed={seed}")
	random.seed(seed)

	# Use pure integer dtype for an integer adder (no fixed-point)
	dtype = channel.Int()

	# Random input token generators (unsigned WIDTH-bit range)
	A_tokens = channel.RandomInt((0, 256), rate=0.8)
	B_tokens = channel.RandomInt((0, 256), rate=0.8)
	S_tokens = channel.Dataless(rate=0.8)

	# Start clock
	cocotb.start_soon(Clock(dut.clk, 10, units="ns").start())

	# Reset
	dut.reset.value = 1
	await RisingEdge(dut.clk)
	await RisingEdge(dut.clk)
	dut.reset.value = 0

	# Wrap DUT ports with Channels
	A = channel.Channel("A", dut.A_valid, dut.A_ready, dut.A_data, dtype=dtype)
	B = channel.Channel("B", dut.B_valid, dut.B_ready, dut.B_data, dtype=dtype)
	S = channel.Channel("S", dut.S_valid, dut.S_ready, dut.S_data, dtype=dtype)

	# Environment
	srcA = channel.Source(A, values=A_tokens, log=log)
	srcB = channel.Source(B, values=B_tokens, log=log)
	sinkS = channel.Sink(S, values=S_tokens, log=log)

	model = dataflow.Add(8, A, B, S, signed=False, log=log)

	for _ in range(10000):
		srcA.cycle()
		srcB.cycle()

		model.cycle()

		sinkS.cycle()

		await RisingEdge(dut.clk)

	log.done()

def test_dataflow_add_expected():
	tb = test.Bench("dataflow_add_expected", "cocotb_dataflow_add")
	tb.source("rtl/dataflow_add_expected.v")
	tb.run()

def test_dataflow_add():
	tb = test.Bench("dataflow_add", "cocotb_dataflow_add")
	tb.source("rtl/dataflow_add.v")
	tb.run()

if __name__ == "__main__":
	test_adder()
