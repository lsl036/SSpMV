import time
import os
import sys

import tensorflow as tf
from tensorflow.keras import Model
from tensorflow.keras.layers import InputLayer, Dense, Conv2D, MaxPooling2D, Flatten,BatchNormalization

import numpy as np

sys.path.append("..")

from utils.load_data import get_train_data
from utils.load_data import get_test_data

from utils.data_setting import *

from compute_metrics import get_acc
from compute_metrics import get_precision

"""
Self defined wide model: should be FFNN : Feed-Forward Neural Network
The standard FFNN is a multi-layer feedforward network with an input, a hidden, and an output layer. 
It can be used to learn and store a large number of mappings between the input and output layers.
"""
class NewWide(Model):
  def __init__(self):
    super(NewWide, self).__init__()
    self.input1 = InputLayer(input_shape=(18,))   # human designed features size
    self.bn = BatchNormalization()
    self.dense1 = Dense(512, activation='relu')
    self.dense2 = Dense(1024, activation='relu')
    self.dense3 = Dense(num_of_labels)            # should be the number of algorithms

  def call(self, x):
    x = self.bn(x)
    x = self.input1(x)
    x = self.dense1(x)
    x = self.dense2(x)
    x = self.dense3(x)
    return x


"""
Self defined deep model: should be CNN : Convolutional Neural Network
Block count is related to non-zero elements on the matrix, 
and normalization restricts their number to within a reasonable range (0~255).
"""
class DeepModel(Model):
  def __init__(self):
    super(DeepModel, self).__init__()
    self.input1 = InputLayer(input_shape=(128, 128, 1))     # Input shape: maybe redefined by 2048*2048 ?
    self.conv1 = Conv2D(16, (3, 3), activation='tanh')      # 16 filters with (3,3) shape
    self.conv2 = Conv2D(16, (5, 5), strides=(2, 2), padding='same',  activation='tanh')
    self.maxpool1 = MaxPooling2D(2, 2)
    self.flatten = Flatten()
    self.dense = Dense(num_of_labels)

  def call(self, x):
    x = self.input1(x)
    x = self.conv1(x)
    x = self.maxpool1(x)
    x = self.conv2(x)
    x = self.maxpool1(x)
    x = self.flatten(x)
    x = self.dense(x)       # 2-layer CNN and flatten, outputsize: should be the number of algorithms
    return x



class NewWideAndDeepConcat(Model):
  def __init__(self):
    super(NewWideAndDeepConcat, self).__init__()
    self.wide = NewWide()
    self.deep = DeepModel()
    self.dense = Dense(num_of_labels)


  def call(self, x):
    x0 = self.wide(x[0])
    x1 = self.deep(x[1]) 
    x =  tf.keras.layers.Concatenate()([x0, x1])
    x = self.dense(x)         # concat size: x0 + x1  --> output size: the number of algorithms 
    return x

# model_path : path to saving model
# image_array: image input
# feat_array : features input
# label_array: label for prediction
def train_new_model_concat(model_path,image_array,feat_array,label_array):
  combined_model = NewWideAndDeepConcat()
  Optimizer = tf.keras.optimizers.Adam(learning_rate=0.001)
  combined_model.compile(optimizer=Optimizer,
                         loss=tf.keras.losses.SparseCategoricalCrossentropy(from_logits=True),
                         metrics=['accuracy'])
  combined_model.fit([feat_array, tf.expand_dims(image_array, -1)], label_array, batch_size=512, epochs=256)  
  #test_loss, test_acc = combined_model.evaluate([feat_array_test, tf.expand_dims(image_array_test, -1)],  label_array_test, verbose=2)
  #print('\nTest accuracy:', test_acc)
  tf.keras.saving.save_model(combined_model, model_path)

def evaluate_new_wide_deep_concat_model(model_path, image_array_test, feat_array_test, label_array_test, test_data, eva_path, res_path):
  loaded_model = tf.keras.saving.load_model(model_path)

  test_loss, test_acc = loaded_model.evaluate([feat_array_test, tf.expand_dims(image_array_test, -1)],  label_array_test, verbose=2)
  f_eva = open(eva_path, "w")                                                   
  f_eva.write("Evaluating Acc:" + str(test_acc) + "\n")
  f_eva.close()

  f_predict = open(res_path, "w")                                                   
  

  for file_ in test_data:
    start = time.time()

    predictions = loaded_model((tf.expand_dims(file_[1],0), tf.expand_dims(tf.expand_dims(file_[0],0), -1)))
    predictions = tf.nn.softmax(predictions)
    idx = tf.math.argmax(predictions[0])

    end = time.time()
    f_predict.write(str(int(idx)) + "\n")
  f_predict.close()


def train_and_get_res():
  training_data_list = "/data1/xionghantao/data/JPDC/data_list/train_list_0.txt"  # 保存的是 dataset matrix name
  test_list = "/data1/xionghantao/data/JPDC/data_list/train_list_0.txt"

  image_array, feat_array, label_array, train_data = get_train_data(training_data_list, image_data, feat_data, label_data, label_file_suffix)
  
  model_path = "/data1/xionghantao/data/JPDC/ml_models/new_wide_deep_concat"
  train_new_model_concat(model_path, image_array, feat_array, label_array)

  image_array, feat_array, label_array, test_data = get_test_data(test_list, image_data, feat_data, label_data, label_file_suffix)
  eva_path = "/data1/xionghantao/data/JPDC/prediction_result/new_wide_deep_concat/evaluate_acc.txt"
  res_path = "/data1/xionghantao/data/JPDC/prediction_result/new_wide_deep_concat/predict_result.txt"
  evaluate_new_wide_deep_concat_model(model_path, image_array, feat_array, label_array, test_data, eva_path, res_path)

  metric_path = "/data1/xionghantao/data/JPDC/prediction_result/new_wide_deep_concat/metrics.res"
  get_acc(test_list, label_data, res_path, metric_path)
  get_precision(test_list, label_data, res_path, num_of_labels, metric_path)


def train_and_get_res_ten_fold():
  train_list_dir = "/data1/xionghantao/data/JPDC/data_list"
  test_list_dir = "/data1/xionghantao/data/JPDC/data_list"
  res_dir = "/data1/xionghantao/data/JPDC/prediction_result/new_wide_deep_concat"
  fold_num = 10
  for i in range(fold_num):
 
    training_list = train_list_dir + "/train_list_" + str(i) + ".txt"
    testing_list = test_list_dir + "/test_list_" + str(i) + ".txt"

    image_array, feat_array, label_array, train_data = get_train_data(training_list, image_data, feat_data, label_data, label_file_suffix)
    model_path = "/data1/xionghantao/data/JPDC/ml_models/new_wide_deep_concat_" + str(i)

    train_new_model_concat(model_path, image_array, feat_array, label_array)

    image_array, feat_array, label_array, test_data = get_test_data(testing_list, image_data, feat_data, label_data, label_file_suffix)
    eva_path = res_dir + "/evaluate_acc_" + str(i) + ".txt"
    res_path = res_dir + "/predict_result_" + str(i) + ".txt"
    
    evaluate_new_wide_deep_concat_model(model_path, image_array, feat_array, label_array, test_data, eva_path, res_path)

    metric_path = res_dir + "/metrics_" + str(i) + ".res"
    get_acc(testing_list, label_data, res_path, metric_path)
    get_precision(testing_list, label_data, res_path, num_of_labels, metric_path)

if __name__ == "__main__":
  train_and_get_res_ten_fold()

 

  
