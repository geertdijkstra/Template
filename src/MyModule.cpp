#include "Template.hpp"


struct MyModule : Module {
	enum ParamId {
		PITCH_PARAM,
		NUM_PARAMS
	};
	enum InputId {
		PITCH_INPUT,
		NUM_INPUTS
	};
	enum OutputId {
		SINE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightId {
		BLINK_LIGHT,
		NUM_LIGHTS
	};

	float phase = 0.f;
	float blinkPhase = 0.f;

	MyModule() {
		// Configure the module
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		// Configure parameters
		// See engine/Param.hpp for config() arguments
		params[PITCH_PARAM].config(-3.f, 3.f, 0.f, "Pitch", " Hz", 2.f, dsp::FREQ_C4);
	}

	void step() override {
		// Implement a simple sine oscillator

		// Get timestep duration
		float deltaTime = app()->engine->getSampleTime();

		// Compute the frequency from the pitch parameter and input
		float pitch = params[PITCH_PARAM].getValue();
		pitch += inputs[PITCH_INPUT].getValue();
		pitch = clamp(pitch, -4.f, 4.f);
		// The default pitch is C4 = 261.6256
		float freq = dsp::FREQ_C4 * std::pow(2.f, pitch);

		// Accumulate the phase
		phase += freq * deltaTime;
		phase = std::fmod(phase, 1.f);

		// Compute the sine output
		float sine = std::sin(2.f * M_PI * phase);
		outputs[SINE_OUTPUT].numChannels = 16;
		outputs[SINE_OUTPUT].setVoltage(5.f * sine);

		// Blink light at 1Hz
		blinkPhase += deltaTime;
		blinkPhase = std::fmod(blinkPhase, 1.f);
		lights[BLINK_LIGHT].setBrightness(blinkPhase < 0.5f ? 1.f : 0.f);
	}

	// For more advanced Module features, see engine/Module.hpp in the Rack API.
	// - dataToJson, dataFromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize: implements custom behavior requested by the user
};


struct MyModuleWidget : ModuleWidget {
	MyModuleWidget(MyModule *module) : ModuleWidget(module) {
		setPanel(SVG::load(asset::plugin(plugin, "res/MyModule.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<Davies1900hBlackKnob>(Vec(28, 87), module, MyModule::PITCH_PARAM));

		addInput(createInput<PJ301MPort>(Vec(33, 186), module, MyModule::PITCH_INPUT));

		addOutput(createOutput<PJ301MPort>(Vec(33, 275), module, MyModule::SINE_OUTPUT));

		addChild(createLight<MediumLight<RedLight>>(Vec(41, 59), module, MyModule::BLINK_LIGHT));
	}
};


// Define the Model with the Module type, ModuleWidget type, and module slug
Model *modelMyModule = createModel<MyModule, MyModuleWidget>("MyModule");
