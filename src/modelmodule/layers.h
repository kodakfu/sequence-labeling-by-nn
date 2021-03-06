#ifndef LAYERS_H_INCLUDE
#define LAYERS_H_INCLUDE

#include <vector>

#include "cnn/nodes.h"
#include "cnn/cnn.h"
#include "cnn/training.h"
#include "cnn/rnn.h"
#include "cnn/lstm.h"
#include "cnn/gru.h"
#include "cnn/dict.h"
#include "cnn/expr.h"
#include "utils/typedeclaration.h"

namespace slnn {

template<typename RNNDerived>
struct BIRNNLayer
{
    RNNDerived *l2r_builder;
    RNNDerived *r2l_builder;
    cnn::Parameters *SOS;
    cnn::Parameters *EOS;
    cnn::expr::Expression SOS_EXP;
    cnn::expr::Expression EOS_EXP;
    cnn::real default_dropout_rate ;

    BIRNNLayer(cnn::Model *model , unsigned nr_rnn_stack_layers, unsigned rnn_x_dim, unsigned  rnn_h_dim ,
                cnn::real default_dropout_rate=0.f);
    ~BIRNNLayer();
    void new_graph(cnn::ComputationGraph &cg);
    void set_dropout(float dropout_rate) ;
    void set_dropout();
    void disable_dropout() ;
    void start_new_sequence();
    void build_graph(const std::vector<cnn::expr::Expression> &X_seq , std::vector<cnn::expr::Expression> &l2r_outputs , 
                     std::vector<cnn::expr::Expression> &r2l_outputs);
};

using BILSTMLayer = BIRNNLayer<cnn::LSTMBuilder>;
using BISimpleRNNLayer = BIRNNLayer<cnn::SimpleRNNBuilder>;
using BIGRULayer = BIRNNLayer<cnn::GRUBuilder>;

struct DenseLayer
{
    cnn::Parameters *w,
        *b;
    cnn::expr::Expression w_exp,
        b_exp;
    DenseLayer(cnn::Model *m , unsigned input_dim , unsigned output_dim );
    ~DenseLayer();
    void new_graph(cnn::ComputationGraph &cg);
    cnn::expr::Expression build_graph(const cnn::expr::Expression &e);
};

struct Merge2Layer
{
    cnn::Parameters *w1,
        *w2,
        *b;
    cnn::expr::Expression w1_exp,
        w2_exp,
        b_exp;
    Merge2Layer(cnn::Model *model , unsigned input1_dim, unsigned input2_dim, unsigned output_dim );
    ~Merge2Layer();
    void new_graph(cnn::ComputationGraph &cg);
    cnn::expr::Expression build_graph(const cnn::expr::Expression &e1, const cnn::expr::Expression &e2);
};

struct Merge3Layer
{
    cnn::Parameters *w1 ,
        *w2 , 
        *w3 ,
        *b;
    cnn::expr::Expression w1_exp,
        w2_exp,
        w3_exp,
        b_exp;
    Merge3Layer(cnn::Model *model ,unsigned input1_dim , unsigned input2_dim , unsigned input3_dim , unsigned output_dim);
    ~Merge3Layer();
    void new_graph(cnn::ComputationGraph &cg);
    cnn::expr::Expression build_graph(const cnn::expr::Expression &e1, const cnn::expr::Expression &e2, const cnn::expr::Expression &e3);
};

struct Merge4Layer
{
    cnn::Parameters *w1 ,
        *w2, 
        *w3,
        *w4,
        *b;
    cnn::expr::Expression w1_exp,
        w2_exp,
        w3_exp,
        w4_exp,
        b_exp;
    Merge4Layer(cnn::Model *model ,unsigned input1_dim , unsigned input2_dim , unsigned input3_dim , unsigned input4_dim, unsigned output_dim);
    ~Merge4Layer();
    void new_graph(cnn::ComputationGraph &cg);
    cnn::expr::Expression build_graph(const cnn::expr::Expression &e1, const cnn::expr::Expression &e2, const cnn::expr::Expression &e3,
        const cnn::expr::Expression &e4);
};

struct MLPHiddenLayer
{
    unsigned nr_hidden_layer;
    std::vector<cnn::Parameters *> w_list;
    std::vector<cnn::Parameters *> b_list;
    std::vector<cnn::expr::Expression> w_expr_list;
    std::vector<cnn::expr::Expression> b_expr_list;
    cnn::real dropout_rate;
    NonLinearFunc *nonlinear_func;
    MLPHiddenLayer(cnn::Model *m, unsigned input_dim, const std::vector<unsigned> &hidden_layer_dim_list, 
        cnn::real dropout_rate=0.f,
        NonLinearFunc *nonlinear_func=cnn::expr::tanh);
    void new_graph(cnn::ComputationGraph &cg);
    cnn::expr::Expression
        build_graph(const cnn::expr::Expression &input_expr);
    void build_graph(const std::vector<cnn::expr::Expression> &input_exprs, std::vector<cnn::expr::Expression> &output_exprs);
};


// ------------------- inline function definition --------------------
// DenseLayer
inline 
void DenseLayer::new_graph(cnn::ComputationGraph &cg)
{
    w_exp = parameter(cg, w);
    b_exp = parameter(cg, b);
}
inline
Expression DenseLayer::build_graph(const cnn::expr::Expression &e)
{
    return affine_transform({ 
       b_exp ,
       w_exp , e 
    });
}

// Merge2Layer 
inline 
void Merge2Layer::new_graph(cnn::ComputationGraph &cg)
{
    b_exp = parameter(cg, b);
    w1_exp = parameter(cg, w1);
    w2_exp = parameter(cg, w2);
}
inline
cnn::expr::Expression Merge2Layer::build_graph(const cnn::expr::Expression &e1, const cnn::expr::Expression &e2)
{
    return affine_transform({
        b_exp ,
        w1_exp , e1,
        w2_exp , e2,
    });
}

// Merge3Layer
inline 
void Merge3Layer::new_graph(cnn::ComputationGraph &cg)
{
    b_exp = parameter(cg, b);
    w1_exp = parameter(cg, w1);
    w2_exp = parameter(cg, w2);
    w3_exp = parameter(cg, w3);

}
inline
cnn::expr::Expression Merge3Layer::build_graph(const cnn::expr::Expression &e1, const cnn::expr::Expression &e2, const cnn::expr::Expression &e3)
{
    return affine_transform({
        b_exp,
        w1_exp, e1 ,
        w2_exp, e2 ,
        w3_exp, e3
    });
}

// Merge4Layer
inline 
void Merge4Layer::new_graph(cnn::ComputationGraph &cg)
{
    b_exp = parameter(cg, b);
    w1_exp = parameter(cg, w1);
    w2_exp = parameter(cg, w2);
    w3_exp = parameter(cg, w3);
    w4_exp = parameter(cg, w4);
}

inline 
cnn::expr::Expression Merge4Layer::build_graph(const cnn::expr::Expression &e1, const cnn::expr::Expression &e2,
    const cnn::expr::Expression &e3, const cnn::expr::Expression &e4)
{
    return affine_transform({
        b_exp,
        w1_exp, e1 ,
        w2_exp, e2 ,
        w3_exp, e3,
        w4_exp, e4
    });
}

// MLPHiddenLayer

inline
void MLPHiddenLayer::new_graph(cnn::ComputationGraph &cg)
{
    for( unsigned i = 0 ; i < nr_hidden_layer; ++i )
    {
        w_expr_list[i] = parameter(cg, w_list[i]);
        b_expr_list[i] = parameter(cg, b_list[i]);
    }
}

inline
cnn::expr::Expression
MLPHiddenLayer::build_graph(const cnn::expr::Expression &input_expr)
{
    cnn::expr::Expression tmp_expr = input_expr;
    for( unsigned i = 0 ; i < nr_hidden_layer; ++i )
    {
        cnn::expr::Expression net_expr = affine_transform({
            b_expr_list[i],
            w_expr_list[i], tmp_expr });
        if( std::abs(dropout_rate - 0.f) > 1e-6 ) { net_expr = cnn::expr::dropout(net_expr, dropout_rate); }
        tmp_expr = (*nonlinear_func)(net_expr);
    }
    return tmp_expr;
}

inline
void MLPHiddenLayer::build_graph(const std::vector<cnn::expr::Expression> &input_exprs,
    std::vector<cnn::expr::Expression> &output_exprs)
{
    unsigned sz = input_exprs.size();
    std::vector<cnn::expr::Expression> tmp_output_exprs(sz);
    for( unsigned i = 0; i < sz; ++i )
    {
        tmp_output_exprs.at(i) = build_graph(input_exprs.at(i));
    }
    swap(output_exprs, tmp_output_exprs);
}

/*****************************
*    Template Implementation
*****************************/

template<typename RNNDerived> 
BIRNNLayer<RNNDerived>::BIRNNLayer(cnn::Model *m , unsigned nr_rnn_stacked_layers, unsigned rnn_x_dim, unsigned rnn_h_dim ,
                                   cnn::real default_dropout_rate)
    : l2r_builder(new RNNDerived(nr_rnn_stacked_layers , rnn_x_dim , rnn_h_dim , m)) ,
    r2l_builder(new RNNDerived(nr_rnn_stacked_layers , rnn_x_dim , rnn_h_dim , m)) ,
    SOS(m->add_parameters({rnn_x_dim})) ,
    EOS(m->add_parameters({rnn_x_dim})) ,
    default_dropout_rate(default_dropout_rate)
{}

template<typename RNNDerived>
BIRNNLayer<RNNDerived>::~BIRNNLayer()
{ 
    delete l2r_builder;
    delete r2l_builder;
}

template <typename RNNDerived>
inline
void BIRNNLayer<RNNDerived>::new_graph(cnn::ComputationGraph &cg)
{
    l2r_builder->new_graph(cg);
    r2l_builder->new_graph(cg);
    SOS_EXP = parameter(cg, SOS);
    EOS_EXP = parameter(cg, EOS);
}

template <typename RNNDerived>
inline
void BIRNNLayer<RNNDerived>::set_dropout(float dropout_rate)
{
    l2r_builder->set_dropout(dropout_rate) ;
    r2l_builder->set_dropout(dropout_rate) ;
}

// SimpleRNNBuilder , GRUBuilder has no function `set_dropout(float)`
template <>
inline
void BIRNNLayer<cnn::SimpleRNNBuilder>::set_dropout(float){ } // empty implementation
template <>
inline
void BIRNNLayer<cnn::GRUBuilder>::set_dropout(float){}


template <typename RNNDerived>
inline
void BIRNNLayer<RNNDerived>::set_dropout()
{
    l2r_builder->set_dropout(default_dropout_rate) ;
    r2l_builder->set_dropout(default_dropout_rate) ;
}
// SimpleRNNBuilder, GRUBuilder has no function `set_dropout(float)`
template <>
inline 
void BIRNNLayer<cnn::SimpleRNNBuilder>::set_dropout(){}
template <>
inline
void BIRNNLayer<cnn::GRUBuilder>::set_dropout(){}

template <typename RNNDerived>
inline
void BIRNNLayer<RNNDerived>::disable_dropout()
{
    l2r_builder->disable_dropout() ;
    r2l_builder->disable_dropout() ;
}
// SimpleRNNBuilder , GRUBulider has no function `disable_dropout()`
template <>
inline 
void BIRNNLayer<cnn::SimpleRNNBuilder>::disable_dropout(){}
template <>
inline
void BIRNNLayer<cnn::GRUBuilder>::disable_dropout(){}


template <typename RNNDerived>
inline
void BIRNNLayer<RNNDerived>::start_new_sequence()
{
    l2r_builder->start_new_sequence();
    r2l_builder->start_new_sequence();
}

template <typename RNNDerived>
inline
void BIRNNLayer<RNNDerived>::build_graph(const std::vector<cnn::expr::Expression> &X_seq, std::vector<cnn::expr::Expression> &l2r_outputs,
                                         std::vector<cnn::expr::Expression> &r2l_outputs)
{
    size_t seq_len = X_seq.size();
    std::vector<cnn::expr::Expression> tmp_l2r_outputs(seq_len),
        tmp_r2l_outputs(seq_len);
    l2r_builder->add_input(SOS_EXP);
    r2l_builder->add_input(EOS_EXP);
    for (int pos = 0; pos < static_cast<int>(seq_len); ++pos)
    {
        tmp_l2r_outputs[pos] = l2r_builder->add_input(X_seq[pos]);
        int reverse_pos = seq_len - pos - 1;
        tmp_r2l_outputs[reverse_pos] = r2l_builder->add_input(X_seq[reverse_pos]);
    }
    swap(l2r_outputs, tmp_l2r_outputs);
    swap(r2l_outputs, tmp_r2l_outputs);
}



} // end of namespace slnn


#endif
