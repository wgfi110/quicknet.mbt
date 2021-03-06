const char* QN_MLP_BaseFl_rcsid =
    "$Header: /u/drspeech/repos/quicknet2/QN_MLP_BaseFl.cc,v 1.2 2011/03/03 00:32:58 davidj Exp $";

/* Must include the config.h file first */
#include <QN_config.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "QN_types.h"
#include "QN_Logger.h"
#include "QN_MLP_BaseFl.h"
#include "QN_fltvec.h"
#include "QN_intvec.h"

#include <iostream>
using namespace std;


// A simple minimum routine

//cz277 - dnn
QN_MLP_BaseFl::QN_MLP_BaseFl(int a_debug, const char* a_dbgname,
                             const char* a_classname,
                             size_t a_size_bunch,
                             size_t a_n_layers,
                             const size_t* a_layer_units)
    : clog(a_debug, a_classname, a_dbgname),
      n_layers(a_n_layers),
      n_weightmats(a_n_layers-1),
      n_sections((n_layers-1) + n_weightmats),
      size_bunch(a_size_bunch)
{
    size_t i;
    float nan = qn_nan_f();

    assert (MAX_LAYERS == MAX_WEIGHTMATS + 1);
   
    //cz277 - dnn
    for (i = 0; i < a_n_layers; ++i)
	layer_units[i] = a_layer_units[i];

    for (i=0; i<MAX_LAYERS; i++)
    {
	layer_size[i] = 0;
	layer_bias[i] = NULL;

	//cz277 - revisit momentum
	bias_delta[i] = NULL;

	neg_bias_learnrate[i] = nan;
    }
    for (i=0; i<MAX_WEIGHTMATS; i++)
    {
	weights_size[i] = 0;
	weights[i] = NULL;
	
	//cz277 - momentum
	weights_delta[i] = NULL;
	
	neg_weight_learnrate[i] = nan;
	if (i>0)
	    backprop_weights[i] = 1;
	else
	    backprop_weights[i] = 0;
    }
    if (n_layers<2 || n_layers > MAX_LAYERS)
    {
	clog.error("Cannot create an MLP with <2 or >%lu layers.",
		 (unsigned long) MAX_LAYERS);
    }
    if (size_bunch == 0)
	clog.error("Cannot use a 0 bunch size.");

    // Set up the per-layer data structures.
    for (i = 0; i<n_layers; i++)
    {
	size_t units = layer_units[i];
        //cerr << units << '\t' << i << endl; //cw564 - mbt
	size_t size = units * size_bunch;
	if (units==0)
	    clog.error("Failed trying to create an MLP with a 0 unit layer.");
	layer_size[i] = size;

	if (i>0)
	{
	    layer_bias[i] = new float [size];
	    qn_copy_f_vf(size, nan, layer_bias[i]); // Fill with NaNs for
						    // safety

	    //cz277 - revisit momentum
	    bias_delta[i] = new float[size];
	    qn_copy_f_vf(size, 0.0, bias_delta[i]);
	}
    }
    // Set up the per-weight-matrix data structures.
    for (i = 0; i<n_weightmats; i++)
    {
	size_t n_weights;

	n_weights = layer_units[i] * layer_units[i+1];
	weights_size[i] = n_weights;
	weights[i] = new float[n_weights];
	
	//cz277 - momentum
	weights_delta[i] = new float[n_weights];
	qn_copy_f_vf(n_weights, 0.0, weights_delta[i]);

	qn_copy_f_vf(n_weights, nan, weights[i]);
    }

    clog.log(QN_LOG_PER_RUN,"Created net, n_layers=%lu bunchsize=%lu.",
	     (unsigned long) n_layers, (unsigned long) size_bunch);
    for (i=0; i<n_layers; i++)
    {
	clog.log(QN_LOG_PER_RUN, "layer_units[%lu]=%lu layer_size[%lu]=%lu.",
		 i, (unsigned long) layer_units[i],
		 i, (unsigned long) layer_size[i]);
    }
}

QN_MLP_BaseFl::~QN_MLP_BaseFl()
{
    size_t i;

    for (i = 0; i<n_layers; i++)
    {
	delete [] layer_bias[i];

	//cz277 - revisit momentum
	delete [] bias_delta[i];
    }
    for (i = 0; i<n_weightmats; i++)
    {
	delete [] weights[i];

	//cz277 - momentum
	delete [] weights_delta[i];
    }
}

size_t
QN_MLP_BaseFl::size_layer(QN_LayerSelector layer) const
{
    size_t r = 0;

    switch(layer)
    {
    case QN_LAYER1:
	r = layer_units[0];
	break;
    case QN_LAYER2:
	r = layer_units[1];
	break;
    case QN_LAYER3:
	if (n_layers>2)
	    r = layer_units[2];
	break;
    case QN_LAYER4:
	if (n_layers>3)
	    r = layer_units[3];
	break;
    case QN_LAYER5:
	if (n_layers>4)
	    r = layer_units[4];
	break;

    //cz277 - dnn
    case QN_LAYER6:
	if (n_layers>5)
	    r = layer_units[5];
	break;
    case QN_LAYER7:
	if (n_layers>6)
	    r = layer_units[6];
	break;
    case QN_LAYER8:
	if (n_layers>7)
	    r = layer_units[7];
	break;
    case QN_LAYER9:
	if (n_layers>8)
	    r = layer_units[8];
	break;
    case QN_LAYER10:
	if (n_layers>9)
	    r = layer_units[9];
	break;
    case QN_LAYER11:
	if (n_layers>10)
	    r = layer_units[10];
	break;
    case QN_LAYER12:
	if (n_layers>11)
	    r = layer_units[11];
	break;
    case QN_LAYER13:
	if (n_layers>12)
	    r = layer_units[12];
	break;
    case QN_LAYER14:
	if (n_layers>13)
	    r = layer_units[13];
	break;
    case QN_LAYER15:
	if (n_layers>14)
	    r = layer_units[14];
	break;
    case QN_LAYER16:
	if (n_layers>15)
	    r = layer_units[15];
	break;
    case QN_LAYER17:
	if (n_layers>16)
	    r = layer_units[16];
	break;
    case QN_LAYER18:
	if (n_layers>17)
	    r = layer_units[17];
	break;
    case QN_LAYER19:
	if (n_layers>18)
	    r = layer_units[18];
	break;
    case QN_LAYER20:
	if (n_layers>19)
	    r = layer_units[19];
	break;

    default:
	break;
    }
    return r;
}




void
QN_MLP_BaseFl::size_section(QN_SectionSelector section, size_t* output_p,
			      size_t* input_p, size_t num_basis) const
{
    // Default to 0 for unavailable sections.
    size_t input = 0;
    size_t output = 0;
    
    switch(section)
    {
    case QN_LAYER12_WEIGHTS:
	output = layer_units[1];
	input = layer_units[0];
	break;
    case QN_LAYER2_BIAS:
	output = layer_units[1];
	input = 1;
	break;
    case QN_LAYER23_WEIGHTS:
	if (n_layers>2)
	{
	    output = layer_units[2];
	    input = layer_units[1];
	}
	break;
    case QN_LAYER3_BIAS:
	if (n_layers>2)
	{
	    output = layer_units[2];
	    input = 1;
	}
	break;
    case QN_LAYER34_WEIGHTS:
	if (n_layers>3)
	{
	    output = layer_units[3];
	    input = layer_units[2];
	}
	break;
    case QN_LAYER4_BIAS:
	if (n_layers>3)
	{
	    output = layer_units[3];
	    input = 1;
	}
	break;
    case QN_LAYER45_WEIGHTS:
	if (n_layers>4)
	{
	    output = layer_units[4];
	    input = layer_units[3] / num_basis; //!!!TODO!!! cw564 - mbt
            //cerr << input << endl; exit(0);
	}
	break;
    case QN_LAYER5_BIAS:
	if (n_layers>4)
	{
	    output = layer_units[4];
	    input = 1;
	}
	break;

    //cz277 - dnn - very ugly
    case QN_LAYER56_WEIGHTS:
	if (n_layers>5)
	{
	    output = layer_units[5];
	    input = layer_units[4];
	}
	break;
    case QN_LAYER6_BIAS:
	if (n_layers>5)
	{
	    output = layer_units[5];
	    input = 1;
	}
	break;
    case QN_LAYER67_WEIGHTS:
	if (n_layers>6)
	{
	    output = layer_units[6];
	    input = layer_units[5];
	}
	break;
    case QN_LAYER7_BIAS:
	if (n_layers>6)
	{
	    output = layer_units[6];
	    input = 1;
	}
	break;
    case QN_LAYER78_WEIGHTS:
	if (n_layers>7)
	{
	    output = layer_units[7];
	    input = layer_units[6];
	}
	break;
    case QN_LAYER8_BIAS:
	if (n_layers>7)
	{
	    output = layer_units[7];
	    input = 1;
	}
	break;
    case QN_LAYER89_WEIGHTS:
	if (n_layers>8)
	{
	    output = layer_units[8];
	    input = layer_units[7];
	}
	break;
    case QN_LAYER9_BIAS:
	if (n_layers>8)
	{
	    output = layer_units[8];
	    input = 1;
	}
	break;
    case QN_LAYER910_WEIGHTS:
	if (n_layers>9)
	{
	    output = layer_units[9];
	    input = layer_units[8];
	}
	break;
    case QN_LAYER10_BIAS:
	if (n_layers>9)
	{
	    output = layer_units[9];
	    input = 1;
	}
	break;
    case QN_LAYER1011_WEIGHTS:
	if (n_layers>10)
	{
	    output = layer_units[10];
	    input = layer_units[9];
	}
	break;
    case QN_LAYER11_BIAS:
	if (n_layers>10)
	{
	    output = layer_units[10];
	    input = 1;
	}
	break;
    case QN_LAYER1112_WEIGHTS:
	if (n_layers>11)
	{
	    output = layer_units[11];
	    input = layer_units[10];
	}
	break;
    case QN_LAYER12_BIAS:
	if (n_layers>11)
	{
	    output = layer_units[11];
	    input = 1;
	}
	break;
    case QN_LAYER1213_WEIGHTS:
	if (n_layers>12)
	{
	    output = layer_units[12];
	    input = layer_units[11];
	}
	break;
    case QN_LAYER13_BIAS:
	if (n_layers>12)
	{
	    output = layer_units[12];
	    input = 1;
	}
	break;
    case QN_LAYER1314_WEIGHTS:
	if (n_layers>13)
	{
	    output = layer_units[13];
	    input = layer_units[12];
	}
	break;
    case QN_LAYER14_BIAS:
	if (n_layers>13)
	{
	    output = layer_units[13];
	    input = 1;
	}
	break;
    case QN_LAYER1415_WEIGHTS:
	if (n_layers>14)
	{
	    output = layer_units[14];
	    input = layer_units[13];
	}
	break;
    case QN_LAYER15_BIAS:
	if (n_layers>14)
	{
	    output = layer_units[14];
	    input = 1;
	}
	break;
    case QN_LAYER1516_WEIGHTS:
	if (n_layers>15)
	{
	    output = layer_units[15];
	    input = layer_units[14];
	}
	break;
    case QN_LAYER16_BIAS:
	if (n_layers>15)
	{
	    output = layer_units[15];
	    input = 1;
	}
	break;
    case QN_LAYER1617_WEIGHTS:
	if (n_layers>16)
	{
	    output = layer_units[16];
	    input = layer_units[15];
	}
	break;
    case QN_LAYER17_BIAS:
	if (n_layers>16)
	{
	    output = layer_units[16];
	    input = 1;
	}
	break;
    case QN_LAYER1718_WEIGHTS:
	if (n_layers>17)
	{
	    output = layer_units[17];
	    input = layer_units[16];
	}
	break;
    case QN_LAYER18_BIAS:
	if (n_layers>17)
	{
	    output = layer_units[17];
	    input = 1;
	}
	break;
    case QN_LAYER1819_WEIGHTS:
	if (n_layers>18)
	{
	    output = layer_units[18];
	    input = layer_units[17];
	}
	break;
    case QN_LAYER19_BIAS:
	if (n_layers>18)
	{
	    output = layer_units[18];
	    input = 1;
	}
	break;
    case QN_LAYER1920_WEIGHTS:
	if (n_layers>19)
	{
	    output = layer_units[19];
	    input = layer_units[18];
	}
	break;
    case QN_LAYER20_BIAS:
	if (n_layers>19)
	{
	    output = layer_units[19];
	    input = 1;
	}
	break;

    default:
	assert(0);
    }
    *input_p = input;
    *output_p = output;
}

size_t
QN_MLP_BaseFl::num_connections() const
{
    size_t i;
    size_t cons = 0;

    // For some reason our number of connections includes biases.
    // This is the same as the other MLP classes.
    // Note the biases are not used in the first layer.
    for (i = 1; i<n_layers; i++)
	cons += layer_units[i];
    for (i=0; i<n_weightmats; i++)
	cons += layer_units[i] * layer_units[i+1];

    return cons;
}


size_t
QN_MLP_BaseFl::get_bunchsize() const
{
    return size_bunch;
}

void
QN_MLP_BaseFl::forward(size_t n_frames, const float* in, float* out, 
        const float * * wgt, const size_t num_basis)
{
    size_t i;
    size_t frames_this_bunch;	// Number of frames to handle this bunch
    size_t n_input = layer_units[0];
    size_t n_output = layer_units[n_layers - 1];

    for (i=0; i<n_frames; i += size_bunch)
    {
	frames_this_bunch = qn_min_zz_z(size_bunch, n_frames - i);
	
	forward_bunch(frames_this_bunch, in, out, wgt, num_basis);
	in += n_input * frames_this_bunch;
	out += n_output * frames_this_bunch;
    }
}

void
QN_MLP_BaseFl::train(size_t n_frames, const float* in, const float* target,
		    float* out, const float * * wgt, const size_t num_basis)
{
    size_t i;
    size_t frames_this_bunch;	// Number of frames to handle this bunch
    size_t n_input = layer_units[0];
    size_t n_output = layer_units[n_layers - 1];


    for (i=0; i<n_frames; i+= size_bunch)
    {
	frames_this_bunch = qn_min_zz_z(size_bunch, n_frames - i);
	train_bunch(frames_this_bunch, in, target, out, wgt, num_basis);
	in += n_input * frames_this_bunch;
	out += n_output * frames_this_bunch;
	target += n_output * frames_this_bunch;
    }
}

void
QN_MLP_BaseFl::set_learnrate(enum QN_SectionSelector which, float learnrate)
{
    size_t i;

    switch(which)
    {
    case QN_LAYER12_WEIGHTS:
	neg_weight_learnrate[0] = -learnrate;
	break;
    case QN_LAYER23_WEIGHTS:
	assert(n_layers>2);
	neg_weight_learnrate[1] = -learnrate;
	break;
    case QN_LAYER34_WEIGHTS:
	assert(n_layers>3);
	neg_weight_learnrate[2] = -learnrate;
	break;
    case QN_LAYER45_WEIGHTS:
	assert(n_layers>4);
	neg_weight_learnrate[3] = -learnrate;
	break;
    
    //cz277 - dnn - very ugly
    case QN_LAYER56_WEIGHTS:
	assert(n_layers>5);
	neg_weight_learnrate[4] = -learnrate;
	break;
    case QN_LAYER67_WEIGHTS:
	assert(n_layers>6);
	neg_weight_learnrate[5] = -learnrate;
	break;
    case QN_LAYER78_WEIGHTS:
	assert(n_layers>7);
	neg_weight_learnrate[6] = -learnrate;
	break;
    case QN_LAYER89_WEIGHTS:
	assert(n_layers>8);
	neg_weight_learnrate[7] = -learnrate;
	break;
    case QN_LAYER910_WEIGHTS:
	assert(n_layers>9);
	neg_weight_learnrate[8] = -learnrate;
	break;
    case QN_LAYER1011_WEIGHTS:
	assert(n_layers>10);
	neg_weight_learnrate[9] = -learnrate;
	break;
    case QN_LAYER1112_WEIGHTS:
	assert(n_layers>11);
	neg_weight_learnrate[10] = -learnrate;
	break;
    case QN_LAYER1213_WEIGHTS:
	assert(n_layers>12);
	neg_weight_learnrate[11] = -learnrate;
	break;
    case QN_LAYER1314_WEIGHTS:
	assert(n_layers>13);
	neg_weight_learnrate[12] = -learnrate;
	break;
    case QN_LAYER1415_WEIGHTS:
	assert(n_layers>14);
	neg_weight_learnrate[13] = -learnrate;
	break;
    case QN_LAYER1516_WEIGHTS:
	assert(n_layers>15);
	neg_weight_learnrate[14] = -learnrate;
	break;
    case QN_LAYER1617_WEIGHTS:
	assert(n_layers>16);
	neg_weight_learnrate[15] = -learnrate;
	break;
    case QN_LAYER1718_WEIGHTS:
	assert(n_layers>17);
	neg_weight_learnrate[16] = -learnrate;
	break;
    case QN_LAYER1819_WEIGHTS:
	assert(n_layers>18);
	neg_weight_learnrate[17] = -learnrate;
	break;
    case QN_LAYER1920_WEIGHTS:
	assert(n_layers>19);
	neg_weight_learnrate[18] = -learnrate;
	break;
    
    case QN_LAYER2_BIAS:
	neg_bias_learnrate[1] = -learnrate;
	break;
    case QN_LAYER3_BIAS:
	assert(n_layers>2);
	neg_bias_learnrate[2] = -learnrate;
	break;
    case QN_LAYER4_BIAS:
	assert(n_layers>3);
	neg_bias_learnrate[3] = -learnrate;
	break;
    case QN_LAYER5_BIAS:
	assert(n_layers>4);
	neg_bias_learnrate[4] = -learnrate;
	break;

    //cz277 - dnn - very ugly
    case QN_LAYER6_BIAS:
	assert(n_layers>5);
	neg_bias_learnrate[5] = -learnrate;
	break;
    case QN_LAYER7_BIAS:
	assert(n_layers>6);
	neg_bias_learnrate[6] = -learnrate;
	break;
    case QN_LAYER8_BIAS:
	assert(n_layers>7);
	neg_bias_learnrate[7] = -learnrate;
	break;
    case QN_LAYER9_BIAS:
	assert(n_layers>8);
	neg_bias_learnrate[8] = -learnrate;
	break;
    case QN_LAYER10_BIAS:
	assert(n_layers>9);
	neg_bias_learnrate[9] = -learnrate;
	break;
    case QN_LAYER11_BIAS:
	assert(n_layers>10);
	neg_bias_learnrate[10] = -learnrate;
	break;
    case QN_LAYER12_BIAS:
	assert(n_layers>11);
	neg_bias_learnrate[11] = -learnrate;
	break;
    case QN_LAYER13_BIAS:
	assert(n_layers>12);
	neg_bias_learnrate[12] = -learnrate;
	break;
    case QN_LAYER14_BIAS:
	assert(n_layers>13);
	neg_bias_learnrate[13] = -learnrate;
	break;
    case QN_LAYER15_BIAS:
	assert(n_layers>14);
	neg_bias_learnrate[14] = -learnrate;
	break;
    case QN_LAYER16_BIAS:
	assert(n_layers>15);
	neg_bias_learnrate[15] = -learnrate;
	break;
    case QN_LAYER17_BIAS:
	assert(n_layers>16);
	neg_bias_learnrate[16] = -learnrate;
	break;
    case QN_LAYER18_BIAS:
	assert(n_layers>17);
	neg_bias_learnrate[17] = -learnrate;
	break;
    case QN_LAYER19_BIAS:
	assert(n_layers>18);
	neg_bias_learnrate[18] = -learnrate;
	break;
    case QN_LAYER20_BIAS:
	assert(n_layers>19);
	neg_bias_learnrate[19] = -learnrate;
	break;
    default:
	assert(0);
	break;
    }
    // Work out if we have whole weight matrices that do not need to
    // be updated due to 0 learning rates.
    for (i=1; i<n_weightmats; i++)
	backprop_weights[i] = 1;
    for (i=1; i<n_weightmats; i++)
    {
	if ((neg_weight_learnrate[i-1]==0.0f) && (neg_bias_learnrate[i]==0.0f))
	    backprop_weights[i] = 0;
	else
	    // If any non-zero learnrates, all following layers need backprop.
	    break;
    }
	
}

float 
QN_MLP_BaseFl::get_learnrate(enum QN_SectionSelector which) const
{
    float res;			// Returned learning rate

    switch(which)
    {
    case QN_LAYER12_WEIGHTS:
	res = -neg_weight_learnrate[0];
	break;
    case QN_LAYER23_WEIGHTS:
	assert(n_layers>2);
	res = -neg_weight_learnrate[1];
	break;
    case QN_LAYER34_WEIGHTS:
	assert(n_layers>3);
	res = -neg_weight_learnrate[2];
	break;
    case QN_LAYER45_WEIGHTS:
	assert(n_layers>4);
	res = -neg_weight_learnrate[3];
	break;

    //cz277 - dnn
    case QN_LAYER56_WEIGHTS:
	assert(n_layers>5);
	res = -neg_weight_learnrate[4];
	break;
    case QN_LAYER67_WEIGHTS:
	assert(n_layers>6);
	res = -neg_weight_learnrate[5];
	break;
    case QN_LAYER78_WEIGHTS:
	assert(n_layers>7);
	res = -neg_weight_learnrate[6];
	break;
    case QN_LAYER89_WEIGHTS:
	assert(n_layers>8);
	res = -neg_weight_learnrate[7];
	break;
    case QN_LAYER910_WEIGHTS:
	assert(n_layers>9);
	res = -neg_weight_learnrate[8];
	break;
    case QN_LAYER1011_WEIGHTS:
	assert(n_layers>10);
	res = -neg_weight_learnrate[9];
	break;
    case QN_LAYER1112_WEIGHTS:
	assert(n_layers>11);
	res = -neg_weight_learnrate[10];
	break;
    case QN_LAYER1213_WEIGHTS:
	assert(n_layers>12);
	res = -neg_weight_learnrate[11];
	break;
    case QN_LAYER1314_WEIGHTS:
	assert(n_layers>13);
	res = -neg_weight_learnrate[12];
	break;
    case QN_LAYER1415_WEIGHTS:
	assert(n_layers>14);
	res = -neg_weight_learnrate[13];
	break;
    case QN_LAYER1516_WEIGHTS:
	assert(n_layers>15);
	res = -neg_weight_learnrate[14];
	break;
    case QN_LAYER1617_WEIGHTS:
	assert(n_layers>16);
	res = -neg_weight_learnrate[15];
	break;
    case QN_LAYER1718_WEIGHTS:
	assert(n_layers>17);
	res = -neg_weight_learnrate[16];
	break;
    case QN_LAYER1819_WEIGHTS:
	assert(n_layers>18);
	res = -neg_weight_learnrate[17];
	break;
    case QN_LAYER1920_WEIGHTS:
	assert(n_layers>19);
	res = -neg_weight_learnrate[18];
	break;

    case QN_LAYER2_BIAS:
	res = -neg_bias_learnrate[1];
	break;
    case QN_LAYER3_BIAS:
	assert(n_layers>2);
	res = -neg_bias_learnrate[2];
	break;
    case QN_LAYER4_BIAS:
	assert(n_layers>3);
	res = -neg_bias_learnrate[3];
	break;
    case QN_LAYER5_BIAS:
	assert(n_layers>4);
	res = -neg_bias_learnrate[4];
	break;
   
    //cz277 - dnn - very ugly
    case QN_LAYER6_BIAS:
	assert(n_layers>5);
	res = -neg_bias_learnrate[5];
	break;
    case QN_LAYER7_BIAS:
	assert(n_layers>6);
	res = -neg_bias_learnrate[6];
	break;
    case QN_LAYER8_BIAS:
	assert(n_layers>7);
	res = -neg_bias_learnrate[7];
	break;
    case QN_LAYER9_BIAS:
	assert(n_layers>8);
	res = -neg_bias_learnrate[8];
	break;
    case QN_LAYER10_BIAS:
	assert(n_layers>9);
	res = -neg_bias_learnrate[9];
	break;
    case QN_LAYER11_BIAS:
	assert(n_layers>10);
	res = -neg_bias_learnrate[10];
	break;
    case QN_LAYER12_BIAS:
	assert(n_layers>11);
	res = -neg_bias_learnrate[11];
	break;
    case QN_LAYER13_BIAS:
	assert(n_layers>12);
	res = -neg_bias_learnrate[12];
	break;
    case QN_LAYER14_BIAS:
	assert(n_layers>13);
	res = -neg_bias_learnrate[13];
	break;
    case QN_LAYER15_BIAS:
	assert(n_layers>14);
	res = -neg_bias_learnrate[14];
	break;
    case QN_LAYER16_BIAS:
	assert(n_layers>15);
	res = -neg_bias_learnrate[15];
	break;
    case QN_LAYER17_BIAS:
	assert(n_layers>16);
	res = -neg_bias_learnrate[16];
	break;
    case QN_LAYER18_BIAS:
	assert(n_layers>17);
	res = -neg_bias_learnrate[17];
	break;
    case QN_LAYER19_BIAS:
	assert(n_layers>18);
	res = -neg_bias_learnrate[18];
	break;
    case QN_LAYER20_BIAS:
	assert(n_layers>19);
	res = -neg_bias_learnrate[19];
	break;
    
    default:
	res = 0;
	assert(0);
	break;
    }
    return res;
}



void
QN_MLP_BaseFl::set_weights(enum QN_SectionSelector which,
			     size_t row, size_t col,
			     size_t n_rows, size_t n_cols,
			     const float* from)
{
    float* start;		// The first element of the relveant weight
				// matrix that we want
    size_t total_cols;		// The total number of columns in the given
				// weight matrix

    start = findweights(which, row, col, n_rows, n_cols,
			&total_cols);
    clog.log(QN_LOG_PER_SUBEPOCH,
	     "Set weights %s @ (%lu,%lu) size (%lu,%lu).",
	     nameweights(which), row, col, n_rows, n_cols);
    qn_copy_mf_smf(n_rows, n_cols, total_cols, from, start);
}


void
QN_MLP_BaseFl::get_weights(enum QN_SectionSelector which,
			      size_t row, size_t col,
			      size_t n_rows, size_t n_cols,
			      float* to)
{
    float* start;		// The first element of the relveant weight
				// matrix that we want
    size_t total_cols;		// The total number of columns in the given
				// weight matrix
    start = findweights(which, row, col, n_rows, n_cols, &total_cols);
    clog.log(QN_LOG_PER_SUBEPOCH,
	     "Get weights %s @ (%lu,%lu) size (%lu,%lu).",
	     nameweights(which), row, col, n_rows, n_cols);
    qn_copy_smf_mf(n_rows, n_cols, total_cols, start, to);
}


const char*
QN_MLP_BaseFl::nameweights(QN_SectionSelector which)
{
    switch(which)
    {
    case QN_LAYER12_WEIGHTS:
	return "layer12_weights";
	break;
    case QN_LAYER23_WEIGHTS:
	return "layer23_weights";
	break;
    case QN_LAYER34_WEIGHTS:
	return "layer34_weights";
	break;
    case QN_LAYER45_WEIGHTS:
	return "layer45_weights";
	break;

    //cz277 - dnn
    case QN_LAYER56_WEIGHTS:
	return "layer56_weights";
	break;
    case QN_LAYER67_WEIGHTS:
	return "layer67_weights";
	break;
    case QN_LAYER78_WEIGHTS:
	return "layer78_weights";
	break;
    case QN_LAYER89_WEIGHTS:
	return "layer89_weights";
	break;
    case QN_LAYER910_WEIGHTS:
	return "layer910_weights";
	break;
    case QN_LAYER1011_WEIGHTS:
	return "layer1011_weights";
	break;
    case QN_LAYER1112_WEIGHTS:
	return "layer1112_weights";
	break;
    case QN_LAYER1213_WEIGHTS:
	return "layer1213_weights";
	break;
    case QN_LAYER1314_WEIGHTS:
	return "layer1314_weights";
	break;
    case QN_LAYER1415_WEIGHTS:
	return "layer1415_weights";
	break;
    case QN_LAYER1516_WEIGHTS:
	return "layer1516_weights";
	break;
    case QN_LAYER1617_WEIGHTS:
	return "layer1617_weights";
	break;
    case QN_LAYER1718_WEIGHTS:
	return "layer1718_weights";
	break;
    case QN_LAYER1819_WEIGHTS:
	return "layer1819_weights";
	break;
    case QN_LAYER1920_WEIGHTS:
	return "layer1920_weights";
	break;

    case QN_LAYER2_BIAS:
	return "layer2_bias";
	break;
    case QN_LAYER3_BIAS:
	return "layer3_bias";
	break;
    case QN_LAYER4_BIAS:
	return "layer4_bias";
	break;
    case QN_LAYER5_BIAS:
	return "layer5_bias";
	break;

    //cz277 - dnn
    case QN_LAYER6_BIAS:
	return "layer6_bias";
	break;
    case QN_LAYER7_BIAS:
	return "layer7_bias";
	break;
    case QN_LAYER8_BIAS:
	return "layer8_bias";
	break;
    case QN_LAYER9_BIAS:
	return "layer9_bias";
	break;
    case QN_LAYER10_BIAS:
	return "layer10_bias";
	break;
    case QN_LAYER11_BIAS:
	return "layer11_bias";
	break;
    case QN_LAYER12_BIAS:
	return "layer12_bias";
	break;
    case QN_LAYER13_BIAS:
	return "layer13_bias";
	break;
    case QN_LAYER14_BIAS:
	return "layer14_bias";
	break;
    case QN_LAYER15_BIAS:
	return "layer15_bias";
	break;
    case QN_LAYER16_BIAS:
	return "layer16_bias";
	break;
    case QN_LAYER17_BIAS:
	return "layer17_bias";
	break;
    case QN_LAYER18_BIAS:
	return "layer18_bias";
	break;
    case QN_LAYER19_BIAS:
	return "layer19_bias";
	break;
    case QN_LAYER20_BIAS:
	return "layer20_bias";
	break;

    default:
	assert(0);
	return "unknown";
	break;
    }
}


float*
QN_MLP_BaseFl::findweights(QN_SectionSelector which,
			     size_t row, size_t col,
			     size_t n_rows, size_t n_cols,
			     size_t* total_cols_p) const
{
// Note - for a weight matrix, inputs are across a row, outputs are
// ..down a column

    float *wp;			// Pointer to bit of weight matrix requested
    size_t total_rows;		// The number of rows in the selected matrix
    size_t total_cols;		// The number of cols in the selected matrix

    size_section(which, &total_rows, &total_cols, mbt_num_basis);
    switch(which)
    {
    case QN_LAYER12_WEIGHTS:
	wp = weights[0];
	break;
    case QN_LAYER23_WEIGHTS:
	assert(n_weightmats>1);
	wp = weights[1];
	break;
    case QN_LAYER34_WEIGHTS:
	assert(n_weightmats>2);
	wp = weights[2];
	break;
    case QN_LAYER45_WEIGHTS:
	assert(n_weightmats>3);
	wp = weights[3];
	break;

    //cz277 - dnn
    case QN_LAYER56_WEIGHTS:
	assert(n_weightmats>4);
	wp = weights[4];
	break;
    case QN_LAYER67_WEIGHTS:
	assert(n_weightmats>5);
	wp = weights[5];
	break;
    case QN_LAYER78_WEIGHTS:
	assert(n_weightmats>6);
	wp = weights[6];
	break;
    case QN_LAYER89_WEIGHTS:
	assert(n_weightmats>7);
	wp = weights[7];
	break;
    case QN_LAYER910_WEIGHTS:
	assert(n_weightmats>8);
	wp = weights[8];
	break;
    case QN_LAYER1011_WEIGHTS:
	assert(n_weightmats>9);
	wp = weights[9];
	break;
    case QN_LAYER1112_WEIGHTS:
	assert(n_weightmats>10);
	wp = weights[10];
	break;
    case QN_LAYER1213_WEIGHTS:
	assert(n_weightmats>11);
	wp = weights[11];
	break;
    case QN_LAYER1314_WEIGHTS:
	assert(n_weightmats>12);
	wp = weights[12];
	break;
    case QN_LAYER1415_WEIGHTS:
	assert(n_weightmats>13);
	wp = weights[13];
	break;
    case QN_LAYER1516_WEIGHTS:
	assert(n_weightmats>14);
	wp = weights[14];
	break;
    case QN_LAYER1617_WEIGHTS:
	assert(n_weightmats>15);
	wp = weights[15];
	break;
    case QN_LAYER1718_WEIGHTS:
	assert(n_weightmats>16);
	wp = weights[16];
	break;
    case QN_LAYER1819_WEIGHTS:
	assert(n_weightmats>17);
	wp = weights[17];
	break;
    case QN_LAYER1920_WEIGHTS:
	assert(n_weightmats>18);
	wp = weights[18];
	break;

    case QN_LAYER2_BIAS:
	wp = layer_bias[1];
	break;
    case QN_LAYER3_BIAS:
	assert(n_layers>2);
	wp = layer_bias[2];
	break;
    case QN_LAYER4_BIAS:
	assert(n_layers>3);
	wp = layer_bias[3];
	break;
    case QN_LAYER5_BIAS:
	assert(n_layers>4);
	wp = layer_bias[4];
	break;

    //cz277 - dnn
    case QN_LAYER6_BIAS:
	assert(n_layers>5);
	wp = layer_bias[5];
	break;
    case QN_LAYER7_BIAS:
	assert(n_layers>6);
	wp = layer_bias[6];
	break;
    case QN_LAYER8_BIAS:
	assert(n_layers>7);
	wp = layer_bias[7];
	break;
    case QN_LAYER9_BIAS:
	assert(n_layers>8);
	wp = layer_bias[8];
	break;
    case QN_LAYER10_BIAS:
	assert(n_layers>9);
	wp = layer_bias[9];
	break;
    case QN_LAYER11_BIAS:
	assert(n_layers>10);
	wp = layer_bias[10];
	break;
    case QN_LAYER12_BIAS:
	assert(n_layers>11);
	wp = layer_bias[11];
	break;
    case QN_LAYER13_BIAS:
	assert(n_layers>12);
	wp = layer_bias[12];
	break;
    case QN_LAYER14_BIAS:
	assert(n_layers>13);
	wp = layer_bias[13];
	break;
    case QN_LAYER15_BIAS:
	assert(n_layers>14);
	wp = layer_bias[14];
	break;
    case QN_LAYER16_BIAS:
	assert(n_layers>15);
	wp = layer_bias[15];
	break;
    case QN_LAYER17_BIAS:
	assert(n_layers>16);
	wp = layer_bias[16];
	break;
    case QN_LAYER18_BIAS:
	assert(n_layers>17);
	wp = layer_bias[17];
	break;
    case QN_LAYER19_BIAS:
	assert(n_layers>18);
	wp = layer_bias[18];
	break;
    case QN_LAYER20_BIAS:
	assert(n_layers>19);
	wp = layer_bias[19];
	break;

    default:
	wp = NULL;
	assert(0);
    }
    assert(row<total_rows);
    assert(row+n_rows<=total_rows);
    assert(col<total_cols);
    assert(col+n_cols<=total_cols);
    wp += col + row * total_cols;
    *total_cols_p = total_cols;
    return(wp);
}



