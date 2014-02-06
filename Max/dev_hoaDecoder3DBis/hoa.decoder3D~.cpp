/**
 * HoaLibrary : A High Order Ambisonics Library
 * Copyright (c) 2012-2013 Julien Colafrancesco, Pierre Guillot, Eliott Paris, CICM, Universite Paris-8.
 * All rights reserved.
 *
 * Website  : http://www.mshparisnord.fr/hoalibrary/
 * Contacts : cicm.mshparisnord@gmail.com
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions may not be sold, nor may they be used in a commercial product or activity.
 *  - Redistributions of source code must retain the above copyright notice, 
 *      this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *  - Neither the name of the CICM nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define MAX_SPEAKER 256

extern "C"
{
#include "ext.h"
#include "ext_obex.h"
#include "ext_path.h"
#include "ext_common.h"
#include "jpatcher_api.h"
#include "jgraphics.h"
#include "jpatcher_syms.h"
#include "ext_dictionary.h"
#include "ext_globalsymbol.h"
#include "ext_parameter.h"
#include "z_dsp.h"
}

#include "../../Sources/hoaLibrary.h"

typedef struct _HoaDecode 
{
	t_pxobject		f_ob;
	double*			f_signals;
	Hoa3D::Decoder* f_decoder;
} t_HoaDecode;

void *HoaDecode_new(t_symbol *s, long argc, t_atom *argv);
void HoaDecode_free(t_HoaDecode *x);
void HoaDecode_assist(t_HoaDecode *x, void *b, long m, long a, char *s);

void HoaDecode_dsp64(t_HoaDecode *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void HoaDecode_perform64(t_HoaDecode *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void HoaDecode_setLoudspeakers(t_HoaDecode *x, t_symbol* s, long argc, t_atom* argv);
void HoaDecode_infos(t_HoaDecode *x);

void *HoaDecode_class;

int C74_EXPORT main(void)
{	
	t_class *c;
	
	c = class_new("hoa.decoder3D~", (method)HoaDecode_new, (method)HoaDecode_free, (long)sizeof(t_HoaDecode), 0L, A_GIMME, 0);
	
	class_addmethod(c, (method)HoaDecode_dsp64,             "dsp64",	A_CANT, 0);
	class_addmethod(c, (method)HoaDecode_assist,            "assist",	A_CANT, 0);
    class_addmethod(c, (method)HoaDecode_setLoudspeakers,	"lscoord",	A_GIMME, 0);
    class_addmethod(c, (method)HoaDecode_infos,	"infos",	A_GIMME, 0);
    
	class_dspinit(c);				
	class_register(CLASS_BOX, c);	
	HoaDecode_class = c;
	
	class_findbyname(CLASS_NOBOX, gensym("hoa.encoder~"));
	return 0;
}

void *HoaDecode_new(t_symbol *s, long argc, t_atom *argv)
{
	t_HoaDecode *x = NULL;
	int order = 4;
    
    x = (t_HoaDecode *)object_alloc((t_class*)HoaDecode_class);
	if(x)
	{
		if(atom_gettype(argv) == A_LONG || atom_gettype(argv) == A_FLOAT)
			order	= atom_getfloat(argv);
		
        x->f_decoder	= new Hoa3D::Decoder(order);
        
        /* DSP Setup */
		dsp_setup((t_pxobject *)x, x->f_decoder->getNumberOfInputs());
		for (int i = 0; i < x->f_decoder->getNumberOfOutputs(); i++)
			outlet_new(x, "signal");
		
		x->f_signals =  new double[x->f_decoder->getNumberOfOutputs() * SYS_MAXBLKSIZE];
		
        attr_args_process(x, argc, argv);
		x->f_ob.z_misc = Z_NO_INPLACE;
	}
    
	return (x);
}

void HoaDecode_dsp64(t_HoaDecode *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	object_method(dsp64, gensym("dsp_add64"), x, HoaDecode_perform64, 0, NULL);
}

void HoaDecode_perform64(t_HoaDecode *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	for(int i = 0; i < sampleframes; i++)
    {
        x->f_decoder->process(ins[0], x->f_signals);
        for(int j = 0; j < numouts; j++)
            outs[j][i] = x->f_signals[j];
    }
}

void HoaDecode_assist(t_HoaDecode *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET)
		sprintf(s,"(Signal) %s",x->f_decoder->getHarmonicsName(a).c_str());
	else
	{
		sprintf(s,"(Signal) channel %ld", a);
	}
}

void HoaDecode_free(t_HoaDecode *x)
{
	dsp_free((t_pxobject *)x);
	delete x->f_decoder;
}

void HoaDecode_setLoudspeakers(t_HoaDecode *x, t_symbol* s, long argc, t_atom* argv)
{
    if(argc > 2 && argv && atom_gettype(argv) == A_LONG && atom_gettype(argv+1) == A_FLOAT && atom_gettype(argv+2) == A_FLOAT)
    {
        x->f_decoder->setLoudspeakerPosition(atom_getlong(argv), atom_getfloat(argv+1), atom_getfloat(argv+2));
    }
}


void HoaDecode_infos(t_HoaDecode *x)
{
	object_post((t_object *)x, "assert !!!", x->f_decoder->getLoudspeakerAzimuth(200));
	/*
    object_post((t_object *)x, "Number Of Ls : %ld", x->f_decoder->getNumberOfOutputs());
    for (int i = 0; i < x->f_decoder->getNumberOfOutputs(); i++)
        object_post((t_object *)x, "Ls  %i : %f %f", i, x->f_decoder->getLoudspeakerAzimuth(i), x->f_decoder->getLoudspeakerElevation(i));
	*/
}

