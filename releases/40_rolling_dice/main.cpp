// Rolling Dice — a program card for the Music Thing Workshop System Computer.
//
// Press the Z switch down to roll a die. For about two seconds all six LEDs
// blink in a random pattern like a tumbling die, then the result (1–6) settles
// as that many lit LEDs on the 2×3 LED grid.
//
// Controls:
//   Z switch Down -> momentary roll button
//
// Outputs:
//   LEDs 0–5 -> during roll: random blinking; after roll: number of lit LEDs
//               equals the rolled value 1–6

#include "ComputerCard.h"

#include "hardware/vreg.h"
#include "pico/rand.h"

class RollingDice : public ComputerCard
{
public:
	RollingDice()
	{
		// Mix the unique flash ID with true hardware entropy so the dice
		// sequence is different every time the card is powered on.
		rng_state_ = FoldSeed(UniqueCardID() ^ static_cast<uint64_t>(get_rand_32()));
	}

	virtual void ProcessSample() override
	{
		// Start a roll on the first sample the Z switch is pressed down.
		// Ignore the press if we are already in the middle of a roll animation.
		if (SwitchChanged() && SwitchVal() == Switch::Down && state_ != State::Rolling)
		{
			StartRoll();
		}

		UpdateState();
		UpdateLeds();
	}

private:
	enum class State { Idle, Rolling, Showing };

	static constexpr int32_t kSampleRate = 48000;
	static constexpr int32_t kLedBrightness = 4095;

	// Roll animation: two seconds of random LED blinking.
	static constexpr int32_t kRollDurationSamples = 2 * kSampleRate;
	// Change the random LED pattern every ~60 ms so the tumbling is visible.
	static constexpr int32_t kLedUpdatePeriodSamples = kSampleRate / 16;

	static uint32_t FoldSeed(uint64_t id)
	{
		uint32_t s = static_cast<uint32_t>(id)
			^ static_cast<uint32_t>(id >> 32)
			^ 0x9e3779b9U;
		return s != 0 ? s : 1U;
	}

	uint32_t Random()
	{
		rng_state_ ^= rng_state_ << 13;
		rng_state_ ^= rng_state_ >> 17;
		rng_state_ ^= rng_state_ << 5;
		return rng_state_;
	}

	void StartRoll()
	{
		state_ = State::Rolling;
		roll_counter_ = kRollDurationSamples;
		led_update_counter_ = 0;
	}

	void UpdateState()
	{
		if (state_ == State::Rolling)
		{
			if (--roll_counter_ <= 0)
			{
				result_ = static_cast<int32_t>(Random() % 6) + 1;
				state_ = State::Showing;
			}
		}
	}

	void UpdateLeds()
	{
		if (state_ == State::Rolling)
		{
			if (--led_update_counter_ <= 0)
			{
				led_update_counter_ = kLedUpdatePeriodSamples;
				// Use the low 6 bits of the xorshift output as a random on/off
				// pattern for the six LEDs.
				led_pattern_ = Random() & 0x3FU;
			}

			for (int32_t i = 0; i < 6; ++i)
			{
				const bool on = (led_pattern_ >> i) & 1U;
				LedBrightness(i, on ? kLedBrightness : 0);
			}
		}
		else
		{
			for (int32_t i = 0; i < 6; ++i)
			{
				LedBrightness(i, i < result_ ? kLedBrightness : 0);
			}
		}
	}

	uint32_t rng_state_ = 1;
	State state_ = State::Idle;
	int32_t result_ = 0;
	int32_t roll_counter_ = 0;
	int32_t led_update_counter_ = 0;
	uint32_t led_pattern_ = 0;
};

int main()
{
	// 144 MHz is a multiple of the 48 kHz sample rate and reduces ADC
	// tonal artifacts; it also leaves plenty of headroom for this card.
	vreg_set_voltage(VREG_VOLTAGE_1_15);
	set_sys_clock_khz(144000, true);

	// Keep the card instance in static storage: the core-0 stack is only
	// 4 KB when both cores are used, and we want headroom.
	static RollingDice card;
	card.Run();
}
