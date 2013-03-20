/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
HoaplugAudioProcessor::HoaplugAudioProcessor()
{
    m_offset_of_loudspeakers    = 0;
    m_distance_of_loudspeakers  = 0.5;
    long anOrder = 1;
    long aNumberOfLoudspeakers = getNumOutputChannels();
    Tools::clip(aNumberOfLoudspeakers, (long)3, (long)64);
    if(aNumberOfLoudspeakers%2 == 0)
        anOrder = aNumberOfLoudspeakers / 2 - 1;
    else
        anOrder = aNumberOfLoudspeakers / 2 - 0.5;
    
    m_ambisonic_tool = new AmbisonicPolyEase(anOrder, getNumInputChannels());
    m_ambisonic_decoder = new AmbisonicDecode(anOrder, aNumberOfLoudspeakers);
    for(int i = 0; i < 64; i++)
    {
        m_sources_abscissa[i] = 0;
        m_sources_ordinate[i] = 0;
    }
    
    for (int i = 0; i < m_number_of_parameters; i++)
    {
        parameters[i].getValueObject().addListener(this);
    }
    
    parameters[0].init ("Offset", UnitDegrees, "Offset of the loudspeakers", 0, -180, 180, 0);
    parameters[1].init ("Distance", UnitGeneric, "Distance of the loudspeakers", 0.5, 0., 1., 0.5);
    
    for(int i = 2; i < m_number_of_parameters; i++)
    {
        if(i%2 == 0)
        {
            char name[256];
            char namo[256];
            sprintf(name, "Source %d X", i/2);
            sprintf(namo, "Abscissa of the source %d", i/2);
            parameters[i].init (name, UnitPan, namo, 0., -1., 1., 0);
        }
        else
        {
            char name[256];
            char namo[256];
            sprintf(name, "Source %d Y", (i-1)/2);
            sprintf(namo, "Ordinate of the source %d", (i-1)/2);
            parameters[i].init (name, UnitPan, namo, 1., -1., 1., 1.);
        }
    }    
}

HoaplugAudioProcessor::~HoaplugAudioProcessor()
{
    for (int i = 0; i < m_number_of_parameters; i++)
    {
        parameters[i].getValueObject().removeListener (this);
    }
    delete m_ambisonic_tool;
    delete m_ambisonic_decoder;
}

//==============================================================================
const String HoaplugAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int HoaplugAudioProcessor::getNumParameters()
{
    return m_number_of_parameters;
}

float HoaplugAudioProcessor::getParameter(int index)
{
    return parameters[index].getNormalisedValue();
}

void HoaplugAudioProcessor::setParameter(int index, float newValue)
{
    parameters[index].setNormalisedValue (newValue);
    if(index == 0)
        m_offset_of_loudspeakers = parameters[0].getValue();
    else if(index == 1)
    {
        m_distance_of_loudspeakers = parameters[1].getValue();
        for (int i = 0; i < getNumInputChannels(); i++) {
            m_ambisonic_tool->setCartesianCoordinates(i, m_sources_abscissa[i] / m_distance_of_loudspeakers, m_sources_ordinate[i] / m_distance_of_loudspeakers);
        }
    }
    else if(index >= 2)
    {
        int indexBis;
        if(index%2 == 0)
        {
            indexBis = ((index-2)/2);
            m_sources_abscissa[indexBis] = parameters[index].getValue();
            m_ambisonic_tool->setCartesianCoordinates(indexBis, parameters[index].getValue() / m_distance_of_loudspeakers, parameters[index+1].getValue() / m_distance_of_loudspeakers);
        }
        else
        {
            indexBis = ((index-3)/2);
            m_sources_ordinate[indexBis] = parameters[index].getValue();
            m_ambisonic_tool->setCartesianCoordinates(indexBis, parameters[index-1].getValue() / m_distance_of_loudspeakers, parameters[index].getValue() / m_distance_of_loudspeakers);
        }
        
        
    }
}

//==============================================================================
float HoaplugAudioProcessor::getScaledParameter (int parameterIndex)
{
    return parameters[parameterIndex].getValue();
}

void HoaplugAudioProcessor::setScaledParameter (int parameterIndex, float newValue)
{
    parameters[parameterIndex].setValue (newValue);
}

float HoaplugAudioProcessor::getParameterMin (int parameterIndex)
{
    return parameters[parameterIndex].getMin();
}

float HoaplugAudioProcessor::getParameterMax (int parameterIndex)
{
    return parameters[parameterIndex].getMax();
}

float HoaplugAudioProcessor::getParameterDefault (int parameterIndex)
{
    return parameters[parameterIndex].getDefault();
}


const String HoaplugAudioProcessor::getParameterName (int index)
{
    return parameters[index].getName();
    return String::empty;
}

const String HoaplugAudioProcessor::getParameterText (int index)
{
    return parameters[index].getTextValueWithSuffix();
}

const String HoaplugAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String(channelIndex + 1);
}

const String HoaplugAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String(channelIndex + 1);
}

bool HoaplugAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool HoaplugAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool HoaplugAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool HoaplugAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool HoaplugAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double HoaplugAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int HoaplugAudioProcessor::getNumPrograms()
{
    return 0;
}

int HoaplugAudioProcessor::getCurrentProgram()
{
    return 0;
}

void HoaplugAudioProcessor::setCurrentProgram (int index)
{
}

const String HoaplugAudioProcessor::getProgramName (int index)
{
    return String::empty;
}

void HoaplugAudioProcessor::changeProgramName (int index, const String& newName)
{
}

void HoaplugAudioProcessor::numChannelsChanged()
{
    delete m_ambisonic_tool;
    delete m_ambisonic_decoder;
    long anOrder = 1;
    long aNumberOfLoudspeakers = getNumOutputChannels();
    Tools::clip(aNumberOfLoudspeakers, (long)3, (long)64);
    if(aNumberOfLoudspeakers%2 == 0)
        anOrder = aNumberOfLoudspeakers / 2 - 1;
    else
        anOrder = aNumberOfLoudspeakers / 2 - 0.5;
    
    m_ambisonic_tool = new AmbisonicPolyEase(anOrder, getNumInputChannels());
    m_ambisonic_decoder = new AmbisonicDecode(anOrder, aNumberOfLoudspeakers);
}
//==============================================================================
void HoaplugAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    if(getNumOutputChannels() != m_ambisonic_decoder->getNumberOfOutputs())
    {
        delete m_ambisonic_tool;
        delete m_ambisonic_decoder;
        long anOrder = 1;
        long aNumberOfLoudspeakers = getNumOutputChannels();
        Tools::clip(aNumberOfLoudspeakers, (long)3, (long)64);
        if(aNumberOfLoudspeakers%2 == 0)
            anOrder = aNumberOfLoudspeakers / 2 - 1;
        else
            anOrder = aNumberOfLoudspeakers / 2 - 0.5;
    
        m_ambisonic_tool = new AmbisonicPolyEase(anOrder, getNumInputChannels());
        m_ambisonic_decoder = new AmbisonicDecode(anOrder, aNumberOfLoudspeakers);
    }
    if(getNumInputChannels() != m_ambisonic_tool->getNumberOfSources())
    {
        m_ambisonic_tool->setNumberOfSources(getNumInputChannels());
    }
    m_ambisonic_tool->setVectorSize(samplesPerBlock);
    m_ambisonic_decoder->setVectorSize(samplesPerBlock);
    for(int i = 0; i < 64; i++)
    {
        free(m_harmonics_vector[i]);
        m_harmonics_vector[i] = new float[samplesPerBlock];
    }
    
}

void HoaplugAudioProcessor::releaseResources()
{
    ;
}

void HoaplugAudioProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    float** channelArray = buffer.getArrayOfChannels();

    m_ambisonic_tool->process(channelArray, m_harmonics_vector);
    m_ambisonic_decoder->process(m_harmonics_vector, channelArray);
}

//==============================================================================
bool HoaplugAudioProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor* HoaplugAudioProcessor::createEditor()
{
    return new HoaplugAudioProcessorEditor (this);
}

//==============================================================================
void HoaplugAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void HoaplugAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HoaplugAudioProcessor();
}

//==============================================================================
void HoaplugAudioProcessor::valueChanged (Value& value)
{
    for (int i = 0; i < m_number_of_parameters; i++)
    {
        PluginParameter& currentParameter = parameters[i];
        if (value.refersToSameSourceAs(currentParameter.getValueObject()))
        {
            sendParamChangeMessageToListeners(i, currentParameter.getNormalisedValue());
        }
    }
}

PluginParameter& HoaplugAudioProcessor::getPluginParameter (int index)
{
    return parameters[index];
}
