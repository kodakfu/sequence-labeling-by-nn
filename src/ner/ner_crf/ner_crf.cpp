#include "ner_crf_model.h"
#include "ner_crf_modelhandler.h"
#include "utils/general.hpp"

using namespace std;
using namespace slnn;
namespace po = boost::program_options;
const string PROGRAM_DESCRIPTION = "Ner-CRF based on CNN Library";


int train_process(int argc, char *argv[], const string &program_name)
{
    string description = PROGRAM_DESCRIPTION + "\n"
        "Training process .\n"
        "using `" + program_name + " train <options>` to train . Training options are as following";
    po::options_description op_des = po::options_description(description);
    op_des.add_options()
        ("training_data", po::value<string>(), "[required] The path to training data")
        ("devel_data", po::value<string>(), "The path to developing data . For validation duration training . Empty for discarding .")
        ("max_epoch", po::value<unsigned>(), "The epoch to iterate for training")
        ("model", po::value<string>(), "Use to specify the model name(path)")
        ("conlleval_script_path", po::value<string>(), "Use to specify the conll evaluation script path")
        ("dropout_rate" , po::value<float>() , "droupout rate for training")
        ("devel_freq", po::value<unsigned>()->default_value(6000), "The frequent(samples number)to validate(if set) . validation will be done after every devel-freq training samples")
        ("do_stat_in_training" , po::value<bool>()->default_value(false) , "1 to calculate the acc during traing ,"
            "which will slow down the training speed . default 0 .")
        ("trivial_report_freq", po::value<unsigned>()->default_value(5000), "Trace frequent during training process")
        ("replace_freq_threshold", po::value<unsigned>()->default_value(1), "The frequency threshold to replace the word to UNK in probability"
            "(eg , if set 1, the words of training data which frequency <= 1 may be "
            " replaced in probability)")
        ("replace_prob_threshold", po::value<float>()->default_value(0.2f), "The probability threshold to replace the word to UNK ."
            " if words frequency <= replace_freq_threshold , the word will"
            " be replace in this probability")
        ("word_embedding_dim", po::value<unsigned>()->default_value(50), "The dimension for dynamic channel word embedding.")
        ("postag_embedding_dim", po::value<unsigned>()->default_value(5), "The dimension for postag embedding.")
        ("ner_embedding_dim" , po::value<unsigned>()->default_value(5) , "The dimension for ner embedding")
        ("nr_lstm_stacked_layer", po::value<unsigned>()->default_value(1), "The number of stacked layers in bi-LSTM.")
        ("lstm_x_dim", po::value<unsigned>()->default_value(50) , "The dimension for LSTM X .")
        ("lstm_h_dim", po::value<unsigned>()->default_value(100), "The dimension for LSTM H.")
        ("emit_hidden_layer_dim", po::value<unsigned>()->default_value(32), "The dimension for tag hidden layer.")
        ("logging_verbose", po::value<int>()->default_value(0), "The switch for logging trace . If 0 , trace will be ignored ,"
                    "else value leads to output trace info.")
        ("help,h", "Show help information.");
    po::variables_map var_map;
    po::store(po::command_line_parser(argc, argv).options(op_des).allow_unregistered().run(), var_map);
    po::notify(var_map);
    if (var_map.count("help"))
    {
        cerr << op_des << endl;
        return 0;
    }
    // trace switch
    if (0 == var_map["logging_verbose"].as<int>())
    {
        boost::log::core::get()->set_filter(
            boost::log::trivial::severity >= boost::log::trivial::debug
        );
    }
    // checking requiring key 
    string training_data_path, devel_data_path , conlleval_script_path ;
    varmap_key_fatal_check(var_map, "training_data",
        "Error : Training data should be specified ! \n"
        "using `" + program_name + " train -h ` to see detail parameters .");
    training_data_path = var_map["training_data"].as<string>();
    
    if (0 == var_map.count("devel_data")) devel_data_path = "";
    else devel_data_path = var_map["devel_data"].as<string>();
    if ("" != devel_data_path)
    {
        varmap_key_fatal_check(var_map, "conlleval_script_path",
            "Eval script path should be specified when doing validation .");
        conlleval_script_path = var_map["conlleval_script_path"].as<string>();
        // Check is evaluation scripts exists 
        if (!FileUtils::exists(conlleval_script_path))
        {
            fatal_error("Eval script is not exists at `" + conlleval_script_path + "`") ;
        }
    }
    
    varmap_key_fatal_check(var_map, "max_epoch",
        "Error : max epoch num should be specified .");
    unsigned max_epoch = var_map["max_epoch"].as<unsigned>();

    varmap_key_fatal_check(var_map , "dropout_rate" ,
        "Error : dropout rate should be specified .") ;
    float dropout_rate = var_map["dropout_rate"].as<float>() ;
    
    // check model path
    string model_path;
    varmap_key_fatal_check(var_map, "model",
        "Error : model path should be specified .");
    model_path = var_map["model"].as<string>();
    if (FileUtils::exists(model_path))
    {
        fatal_error("Error : model file `" + model_path + "` has already exists .");
    }
    ofstream model_os(model_path);
    if (!model_os)
    {
        BOOST_LOG_TRIVIAL(fatal) << "failed to open model path at '" << model_path << "'. \n Exit !";
        return -1;
    }
    // some key which has default value
    bool is_do_stat_in_training = var_map["do_stat_in_training"].as<bool>() ;
    unsigned devel_freq = var_map["devel_freq"].as<unsigned>();
    unsigned trivial_report_freq = var_map["trivial_report_freq"].as<unsigned>();

    unsigned replace_freq_threshold = var_map["replace_freq_threshold"].as<unsigned>();
    float replace_prob_threshold = var_map["replace_prob_threshold"].as<float>();
    // others will be processed flowing 
    
    // Init 
    cnn::Initialize(argc, argv, 1234); 
    NERCRFModelHandler model_handler;

    // reading traing data , get word dict size and output tag number
    // -> set replace frequency for word_dict_wrapper
    model_handler.set_unk_replace_threshold(replace_freq_threshold, replace_prob_threshold);
    
    ifstream train_is(training_data_path);
    if (!train_is) {
        fatal_error("Error : failed to open training: `" + training_data_path + "` .");
    }
    vector<IndexSeq> sents , 
        postag_seqs ,
        ner_seqs ;
    model_handler.read_training_data_and_build_dicts(train_is, sents , 
        postag_seqs , ner_seqs);
    train_is.close();
    // set model structure param 
    model_handler.finish_read_training_data(var_map);
    
    // build model structure
    model_handler.build_model(); // passing the var_map to specify the model structure
    
    // reading developing data
    vector<IndexSeq> dev_sents, *p_dev_sents ,
        dev_postag_seqs, *p_dev_postag_seqs ,
        dev_ner_seqs , *p_dev_ner_seqs;
    if ("" != devel_data_path)
    {
        std::ifstream devel_is(devel_data_path);
        if (!devel_is) {
            fatal_error("Error : failed to open devel file: `" + devel_data_path + "`");
        }
        model_handler.read_devel_data(devel_is, dev_sents , 
            dev_postag_seqs , dev_ner_seqs);
        devel_is.close();
        p_dev_sents = &dev_sents;
        p_dev_postag_seqs = &dev_postag_seqs;
        p_dev_ner_seqs = &dev_ner_seqs;
    }
    else
    {
        p_dev_sents = p_dev_postag_seqs = p_dev_ner_seqs =  nullptr;
    }

    // Train 
    model_handler.train(&sents , &postag_seqs , &ner_seqs , 
        max_epoch, 
        dropout_rate , 
        p_dev_sents , p_dev_postag_seqs , p_dev_ner_seqs , 
        conlleval_script_path , 
        devel_freq , 
        is_do_stat_in_training ,
        trivial_report_freq);

    // save model
    model_handler.save_model(model_os);
    model_os.close();
    return 0;
}

int devel_process(int argc, char *argv[], const string &program_name)
{
    string description = PROGRAM_DESCRIPTION + "\n"
        "Validation(develop) process "
        "using `" + program_name + " devel <options>` to validate . devel options are as following";
    po::options_description op_des = po::options_description(description);
    // set params to receive the arguments 
    string devel_data_path, model_path, eval_script_path;
    op_des.add_options()
        ("devel_data", po::value<string>(&devel_data_path), "The path to validation data .")
        ("model", po::value<string>(&model_path), "Use to specify the model name(path)")
        ("conlleval_script_path", po::value<string>(&eval_script_path)->default_value(string("./ner_eval.sh")),
            "Use to specify the conll evaluation script path")
        ("help,h", "Show help information.");
    po::variables_map var_map;
    po::store(po::command_line_parser(argc, argv).options(op_des).allow_unregistered().run(), var_map);
    po::notify(var_map);
    if (var_map.count("help"))
    {
        cerr << op_des << endl;
        return 0;
    }

    varmap_key_fatal_check(var_map, "devel_data", "Error : validation(develop) data should be specified !");
    varmap_key_fatal_check(var_map, "model", "Error : model path should be specified !");
    // Check is evaluation scripts exists 
    if (!FileUtils::exists(eval_script_path)) fatal_error("Error : eval script is not exists at `" + eval_script_path + "`");
    
    // Init 
    cnn::Initialize(argc, argv, 1234);
    NERCRFModelHandler model_handler;
    // Load model 
    ifstream model_is(model_path);
    if (!model_is)
    {
        fatal_error("Error : failed to open model path at '" + model_path + "' .");
    }
    model_handler.load_model(model_is);
    model_is.close();

    // read validation(develop) data
    std::ifstream devel_is(devel_data_path);
    if (!devel_is) {
        fatal_error("Error : failed to open devel file: `" + devel_data_path + "`");
    }

    // read devel data
    vector<IndexSeq> sents,
        postag_seqs ,
        ner_seqs ;
    model_handler.read_devel_data(devel_is, sents , postag_seqs , ner_seqs);
    devel_is.close();

    // devel
    model_handler.devel(&sents , &postag_seqs, &ner_seqs , eval_script_path); // Get the same result , it is OK .
    
    return 0;
}


int predict_process(int argc, char *argv[], const string &program_name)
{
    string description = PROGRAM_DESCRIPTION + "\n"
        "Predict process ."
        "using `" + program_name + " predict <options>` to predict . predict options are as following";
    po::options_description op_des = po::options_description(description);
    string raw_data_path, output_path, model_path;
    op_des.add_options()
        ("raw_data", po::value<string>(&raw_data_path), "The path to raw data(It should be segmented) .")
        ("output", po::value<string>(&output_path), "The path to storing result . using `stdout` if not specified .")
        ("model", po::value<string>(&model_path), "Use to specify the model name(path)")
        ("help,h", "Show help information.");
    po::variables_map var_map;
    po::store(po::command_line_parser(argc, argv).options(op_des).allow_unregistered().run(), var_map);
    po::notify(var_map);
    if (var_map.count("help"))
    {
        cerr << op_des << endl;
        return 0;
    }

    //set params 
    
    varmap_key_fatal_check(var_map, "raw_data", "raw_data path should be specified .");
    
    if (output_path == "")
    {
        BOOST_LOG_TRIVIAL(info) << "no output is specified . using stdout .";
    }

    varmap_key_fatal_check(var_map, "model", "Error : model path should be specified ! ");
    
    // Init 
    cnn::Initialize(argc, argv, 1234);
    NERCRFModelHandler model_handler ;

    // load model 
    ifstream is(model_path);
    if (!is)
    {
        fatal_error("Error : failed to open model path at '" + model_path + "' . ");
    }
    model_handler.load_model(is);
    is.close();

    // open raw_data
    ifstream raw_is(raw_data_path);
    if (!raw_is)
    {
        fatal_error("Error : failed to open raw data at '" + raw_data_path + "'");
    }

    // open output 
    if ("" == output_path)
    {
        model_handler.predict(raw_is, cout); // using `cout` as output stream 
        raw_is.close();
    }
    else
    {
        ofstream os(output_path);
        if (!os)
        {
            raw_is.close();
            fatal_error("Error : failed open output file at : `" +  output_path + "`.");
        }
        model_handler.predict(raw_is, os);
        os.close();
    }
    return 0;
}


int main(int argc, char *argv[])
{
    string usage = PROGRAM_DESCRIPTION + "\n"
        "usage : " + string(argv[0]) + " [ train | devel | predict ] <options> \n"
        "using  `" + string(argv[0]) + " [ train | devel | predict ] -h` to see details for specify task\n";
    if (argc <= 1)
    {
        cerr << usage;
        return -1;
    }
    else if (string(argv[1]) == "train") return train_process(argc - 1, argv + 1, argv[0]);
    else if (string(argv[1]) == "devel") return devel_process(argc - 1, argv + 1, argv[0]);
    else if (string(argv[1]) == "predict") return predict_process(argc - 1, argv + 1, argv[0]);
    else
    {
        cerr << "unknown mode : " << argv[1] << "\n"
            << usage;
        return -1;
    }
}
