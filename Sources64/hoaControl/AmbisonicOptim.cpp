/*
 * Copyright (C) 2012 Julien Colafrancesco & Pierre Guillot, Universite Paris 8
 * 
 * This library is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU Library General Public License as published 
 * by the Free Software Foundation; either version 2 of the License.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public 
 * License for more details.
 *
 * You should have received a copy of the GNU Library General Public License 
 * along with this library; if not, write to the Free Software Foundation, 
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include "cicmTools.h"
#include "AmbisonicOptim.h"

AmbisonicOptim::AmbisonicOptim(long anOrder, std::string anOptimMode, long aVectorSize)
{
	m_order					= Tools::clip_min(anOrder, (long)1);
	m_number_of_harmonics	= m_order * 2 + 1;
	m_number_of_inputs		= m_number_of_harmonics;
	m_number_of_outputs		= m_number_of_harmonics;

	m_optim_vector		= new double[m_number_of_harmonics];
	computeIndex();
	setVectorSize(aVectorSize);
	setOptimMode(anOptimMode);
}

long AmbisonicOptim::getOrder()
{
	return m_order;
}

long AmbisonicOptim::getNumberOfHarmonics()
{
	return m_number_of_harmonics;
}

long AmbisonicOptim::getNumberOfInputs()
{
	return m_number_of_inputs;
}

long AmbisonicOptim::getNumberOfOutputs()
{
	return m_number_of_outputs;
}

long AmbisonicOptim::getVectorSize()
{
	return m_vector_size;
}

std::string AmbisonicOptim::getMode()
{
	return m_optim_mode;
}

void AmbisonicOptim::computeIndex()
{
	m_index_of_harmonics	= new int[m_number_of_harmonics ];
	m_index_of_harmonics[0] = 0;
	for(int i = 1; i < m_number_of_harmonics; i++)
	{
		m_index_of_harmonics[i] = (i - 1) / 2 + 1;
		if (i % 2 == 1) 
			m_index_of_harmonics[i] = - m_index_of_harmonics[i];
	}
}
void AmbisonicOptim::setOptimMode(std::string anOptim)
{
	if(anOptim != m_optim_mode)
	{
		if(anOptim == "inPhase")
			computeInPhaseOptim();
		else if(anOptim == "maxRe")
			computeReOptim();
		else
			computeBasicOptim();
	}
}

void AmbisonicOptim::computeBasicOptim()
{
	m_optim_mode = "basic"; 
	for (int i = 0; i < m_number_of_harmonics; i++) 
		m_optim_vector[i] = 1.;
}

void AmbisonicOptim::computeInPhaseOptim()
{
	m_optim_mode = "inPhase"; 
	for (int i = 0; i < m_number_of_harmonics; i++) 
	{
		if (i == 0) 
			m_optim_vector[i] = 1.;
		else 
			m_optim_vector[i] = pow(gsl_sf_fact(m_order), 2) / ( gsl_sf_fact(m_order+abs(m_index_of_harmonics[i])) * gsl_sf_fact(m_order-abs(m_index_of_harmonics[i])));
	}
}

void AmbisonicOptim::computeReOptim()
{
	m_optim_mode = "maxRe";
	for (int i = 0; i < m_number_of_harmonics; i++) 
	{
		if (i == 0) 
			m_optim_vector[i] = 1.;
		else 
			m_optim_vector[i] = cos(abs(m_index_of_harmonics[i]) * M_PI / (2*m_order+2));
	}
	
}

void AmbisonicOptim::setVectorSize(int aVectorSize)
{
	m_vector_size = Tools::clip_power_of_two(aVectorSize);
}

AmbisonicOptim::~AmbisonicOptim()
{
	free(m_index_of_harmonics);
	free(m_optim_vector);
}

