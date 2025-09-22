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

from models import serial

@cocotb.test()
async def cocotb_serial_add(dut):
	params = test.Params()
	log = test.Logger()

	# Seed logging
	seed = random.randint(-1_000_000_000, 1_000_000_000)
	log.info(f"seed={seed}")
	random.seed(seed)

	# Use pure integer dtype for an integer adder (no fixed-point)
	dtype = channel.Int()

	# Random input token generators (unsigned WIDTH-bit range)
	Ac_tokens = channel.RandomInt((0, 2),  rate=0.8)
	Ad_tokens = channel.RandomInt((0, 16), rate=0.8)
	Bc_tokens = channel.RandomInt((0, 2),  rate=0.8)
	Bd_tokens = channel.RandomInt((0, 16), rate=0.8)
	Sc_tokens = channel.Dataless(rate=0.8)
	Sd_tokens = channel.Dataless(rate=0.8)

	# Start clock
	cocotb.start_soon(Clock(dut.clk, 10, units="ns").start())

	# Reset
	dut.reset.value = 1
	await RisingEdge(dut.clk)
	await RisingEdge(dut.clk)
	dut.reset.value = 0

	# Wrap DUT ports with Channels
	Ac = channel.Channel("Ac", dut.Ac_valid, dut.Ac_ready, dut.Ac_data, dtype=dtype)
	Ad = channel.Channel("Ad", dut.Ad_valid, dut.Ad_ready, dut.Ad_data, dtype=dtype)
	Bc = channel.Channel("Bc", dut.Bc_valid, dut.Bc_ready, dut.Bc_data, dtype=dtype)
	Bd = channel.Channel("Bd", dut.Bd_valid, dut.Bd_ready, dut.Bd_data, dtype=dtype)
	Sc = channel.Channel("Sc", dut.Sc_valid, dut.Sc_ready, dut.Sc_data, dtype=dtype)
	Sd = channel.Channel("Sd", dut.Sd_valid, dut.Sd_ready, dut.Sd_data, dtype=dtype)

	# Environment
	srcAc = channel.Source(Ac, values=Ac_tokens, log=log)
	srcAd = channel.Source(Ad, values=Ad_tokens, log=log)
	srcBc = channel.Source(Bc, values=Bc_tokens, log=log)
	srcBd = channel.Source(Bd, values=Bd_tokens, log=log)
	sinkSc = channel.Sink(Sc, values=Sc_tokens, log=log)
	sinkSd = channel.Sink(Sd, values=Sd_tokens, log=log)

	model = serial.Add(Ac, Ad, Bc, Bd, Sc, Sd, log)

	for _ in range(10000):
		srcAc.cycle()
		srcAd.cycle()
		srcBc.cycle()
		srcBd.cycle()

		model.cycle()

		sinkSc.cycle()
		sinkSd.cycle()

		await RisingEdge(dut.clk)

	log.done()

def test_serial_add_expected():
	tb = test.Bench("serial_add_expected", "cocotb_serial_add")
	tb.source("rtl/serial_add_expected.v")
	tb.run()

def test_serial_add():
	tb = test.Bench("serial_add", "cocotb_serial_add")
	tb.source("rtl/serial_add.v")
	tb.run()

if __name__ == "__main__":
	test_adder()
