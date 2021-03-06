#include "pos_single_classification_model.h"
#include "modelmodule/hyper_layers.h"

namespace slnn{

POSSingleClassificationModel::POSSingleClassificationModel()
    :SingleInputModel() ,
    word_dict(input_dict) ,
    tag_dict(output_dict)
{}

POSSingleClassificationModel::~POSSingleClassificationModel(){}

void POSSingleClassificationModel::set_model_param(const boost::program_options::variables_map &var_map)
{
    assert(word_dict.is_frozen() && tag_dict.is_frozen()) ;

    word_embedding_dim = var_map["word_embedding_dim"].as<unsigned>() ;
    lstm_nr_stacked_layer = var_map["nr_lstm_stacked_layer"].as<unsigned>() ;
    lstm_h_dim = var_map["lstm_h_dim"].as<unsigned>() ;
    hidden_dim = var_map["tag_layer_hidden_dim"].as<unsigned>() ;

    dropout_rate = var_map["dropout_rate"].as<cnn::real>() ;
    word_dict_size = word_dict.size() ;
    output_dim = tag_dict.size() ;
}
void POSSingleClassificationModel::build_model_structure()
{
    m = new cnn::Model() ;
    input_layer = new Input1(m, word_dict_size, word_embedding_dim) ;
    bilstm_layer = new BILSTMLayer(m, lstm_nr_stacked_layer, word_embedding_dim, lstm_h_dim, dropout_rate) ;
    output_layer = new SimpleOutput(m, lstm_h_dim, lstm_h_dim, hidden_dim, output_dim) ;
}

void POSSingleClassificationModel::print_model_info()
{
    BOOST_LOG_TRIVIAL(info) << "---------------- Single Input Classification Model -----------------\n"
        << "vocabulary size : " << word_dict_size << " with dimension : " << word_embedding_dim << "\n"
        << "bi-lstm x dim : " << word_embedding_dim << " , h dim : " << lstm_h_dim
        << " , stacked layer num : " << lstm_nr_stacked_layer << "\n"
        << "tag hidden layer dim : " << hidden_dim << "\n"
        << "output dim : " << output_dim ;
}

void POSSingleClassificationModel::save_model(std::ostream &os)
{
    BOOST_LOG_TRIVIAL(info) << "saving model ...";
    boost::archive::text_oarchive to(os) ;
    to << word_embedding_dim << word_dict_size
        << lstm_h_dim << lstm_nr_stacked_layer
        << hidden_dim
        << output_dim 
        << dropout_rate ;
    to << word_dict << tag_dict ;
    to << *m ;
    BOOST_LOG_TRIVIAL(info) << "save model done .";
}

void POSSingleClassificationModel::load_model(std::istream &is)
{
    BOOST_LOG_TRIVIAL(info) << "loading model ...";
    boost::archive::text_iarchive ti(is) ;
    ti >> word_embedding_dim >> word_dict_size
        >> lstm_h_dim >> lstm_nr_stacked_layer
        >> hidden_dim
        >> output_dim
        >> dropout_rate ;
    ti >> word_dict >> tag_dict ;
    assert(word_dict_size == word_dict.size() && output_dim == tag_dict.size()) ;
    build_model_structure() ;
    ti >> *m ;
    BOOST_LOG_TRIVIAL(info) << "load model done .";
}

}
