/*
* xtechnical_analysis - Technical analysis C++ library
*
* Copyright (c) 2018 Elektro Yar. Email: git.electroyar@gmail.com
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#ifndef XTECHNICAL_INDICATORS_HPP_INCLUDED
#define XTECHNICAL_INDICATORS_HPP_INCLUDED

#include "xtechnical_dft.hpp"
#include "xtechnical_correlation.hpp"
#include "xtechnical_normalization.hpp"
#include "xtechnical_common.hpp"

#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>

#define INDICATORSEASY_DEF_RING_BUFFER_SIZE 1024

namespace xtechnical_indicators {
    using namespace xtechnical_common;

    /** \brief Посчитать простую скользящую среднюю (SMA)
     *
     * Данная функция для расчетов использует последние N = period значений
     * \param input массив значений
     * \param output значение SMA
     * \param period период SMA
     * \param start_pos начальная позиция в массиве
     * \return вернет 0 в случае успеха
     */
    template <typename T1, typename T2>
    int calculate_sma(
            T1 &input,
            T2 &output,
            const size_t period,
            const size_t start_pos = 0) {
        if(input.size() <= start_pos + period) return INVALID_PARAMETER;
        using NumType = typename T1::value_type;
        auto sum = std::accumulate(
            input.begin() + start_pos,
            input.begin() + start_pos + period,
            NumType(0));
        output = sum / (T2)period;
        return OK;
    }

    /** \brief Расчитать стандартное отклонение
     * \param input входные данные индикатора
     * \param output стандартное отклонение
     * \param period период STD
     * \param start_pos начальная позиция в массиве
     * \return вернет 0 в случае успеха
     */
    template<typename T1, typename T2>
    int calculate_std_dev(
            T1 &input,
            T2 &output,
            const size_t period,
            const size_t start_pos = 0) {
        if(input.size() < start_pos + period)return INVALID_PARAMETER;
        using NumType = typename T1::value_type;
        auto mean = std::accumulate(
            input.begin() + start_pos,
            input.begin() + start_pos + period,
            NumType(0));
        mean /= (NumType)period;
        double _std_dev = 0;
        for(int i = 0; i < (int)input.size(); i++) {
            double diff = (input[i] - mean);
            _std_dev +=  diff * diff;
        }
        output = std::sqrt(_std_dev / (T2)(period - 1));
        return OK;
    }

    /** \brief Расчитать стандартное отклонение и среднее значение
     * \param input входные данные индикатора
     * \param output стандартное отклонение
     * \param period период STD
     * \param start_pos начальная позиция в массиве
     * \return вернет 0 в случае успеха
     */
    template<typename T1, typename T2>
    int calculate_std_dev_and_mean(
            T1 &input,
            T2 &std_dev,
            T2 &mean,
            const size_t period,
            const size_t start_pos = 0) {
        if(input.size() < start_pos + period)return INVALID_PARAMETER;
        using NumType = typename T1::value_type;
        mean = (T2)std::accumulate(
            input.begin() + start_pos,
            input.begin() + start_pos + period,
            NumType(0));
        mean /= (NumType)period;
        double _std_dev = 0;
        for(int i = 0; i < (int)input.size(); i++) {
            double diff = (input[i] - mean);
            _std_dev +=  diff * diff;
        }
        std_dev = std::sqrt(_std_dev / (T2)(period - 1));
        return OK;
    }

    /** \brief Кольцевой буфер
     */
    template <typename T>
    class RingBuffer {
    public:
        //T data[SIZE];
        std::vector<T> data;
    private:
        size_t pos = 0;
        size_t data_size = 0;
        size_t read_count = 0;
    public:
        RingBuffer() {};

        RingBuffer(const size_t &size) {
            data.resize(size);
            data_size = size;
        }

        void resize(const size_t &size) {
            data.resize(size);
            data_size = size;
        }

        inline size_t size() {
            return data_size;
        }

        inline size_t count() {
            return read_count;
        }

        void push(const T &value) {
            data[pos] = value;
            pos =(pos + 1) % data_size;
            if(read_count < data_size) read_count++;
        }

        bool empty() {
            if(read_count > 0) return false;
            return true;
        }

        void clear() {
            pos = 0;
            read_count = 0;
        }

        inline T& operator[] (size_t i) {
            return data[(pos + i) % data_size];
        }

        inline const T operator[] (size_t i)const {
            return data[(pos + i) % data_size];
        }

        std::vector<T> get_data() {
            std::vector<T> temp(data_size);
            for(int i = 0; i < data_size; ++i) {
                temp[i] = (data[(pos + i) % data_size]);
            }
            return temp;
        }

    #if(0)
        inline std::vector<T> get_raw_data() {
                return data;
        }
    #endif

        inline double get_sum() {
            double sum = 0;
            for(size_t i = 0; i < data_size; ++i) {
                sum += (data[(pos + i) % data_size]);
            }
            return sum;
        }
    };

    /** \brief Простая скользящая средняя
     */
    template <typename T>
    class SMA {
    private:
        RingBuffer<T> data_;
        T last_data_ = 0;
        size_t period_ = 0;
        size_t pos_ = 0;
    public:
        SMA() {};

        /** \brief Инициализировать простую скользящую среднюю
         * \param period период
         */
        SMA(const size_t period) : period_(period) {
            data_.resize(period_);
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &in, T &out) {
            if(period_ == 0) {
                out = 0;
                return NO_INIT;
            }
            if(data_.count() < period_) {
                data_.push(in);
                if(data_.count() == period_) {
                    T sum = data_.get_sum();
                    last_data_ = sum;
                    out = sum / (T)period_;
                    pos_ = 0;
                    return OK;
                }
            } else {
                last_data_ = last_data_ + (in - data_[0]);
                data_.push(in);
                out = last_data_/(T)period_;
                return OK;
            }
            out = 0;
            return INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &in) {
            if(period_ == 0) return NO_INIT;
            if(data_.count() < period_) {
                data_.push(in);
                if(data_.count() == period_) {
                    T sum = data_.get_sum();
                    last_data_ = sum;
                    pos_ = 0;
                    return OK;
                }
            } else {
                last_data_ = last_data_ + (in - data_[0]);
                data_.push(in);
                return OK;
            }
            return INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T &in, T &out) {
            if(period_ == 0) {
                out = 0;
                return NO_INIT;
            }
            RingBuffer<T> _data = data_;
            if(_data.count() < period_) {
                _data.push(in);
                if(_data.count() == period_) {
                    T sum = _data.get_sum();
                    out = sum / (T)period_;
                    return OK;
                }
            } else {
                out = (last_data_ - data_[0] + in)/(T)period_;
                return OK;
            }
            out = 0;
            return INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Очистить данные индикатора
         */
        void clear() {
            data_.clear();
            pos_ = 0;
        }
    };


	/** \brief Линия задержки
     */
    template <typename T>
    class DelayLine {
    private:
        std::vector<T> data_;
        size_t period_ = 0;
    public:

        DelayLine() {};

        /** \brief Конструктор линии задержки
         * \param period период
         */
        DelayLine(const size_t period) : period_(period) {
            data_.reserve(period_);
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in, T &out) {
            if(period_ == 0) {
                out = in;
                return OK;
            }
            if(data_.size() < (size_t)period_) {
                data_.push_back(in);
                if(data_.size() == (size_t)period_) {
                    out = data_.front();
                    return OK;
                }
            } else {
                data_.push_back(in);
                data_.erase(data_.begin());
                out = data_.front();
                return OK;
            }
            out = in;
            return INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Протестировать индикатор
         *
         * Данный метод отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in, T &out) {
            if(period_ == 0) {
                out = in;
                return OK;
            }
            std::vector<T> _data = data_;
            if(_data.size() < (size_t)period_) {
                _data.push_back(in);
                if(_data.size() == (size_t)period_) {
                    out = _data.front();
                    return OK;
                }
            } else {
                _data.push_back(in);
                _data.erase(_data.begin());
                out = _data.front();
                return OK;
            }
            out = in;
            return INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Очистить данные индикатора
         */
        void clear() {
            data_.clear();
        }
    };

    /** \brief Кумулятивное скользящее среднее
     */
    template <typename T>
    class CMA {
    private:
        uint64_t n = 0;
        uint64_t tn = 0;
        T sum = 0;
        bool is_test = false;
    public:
        CMA() {};

        double update(const T in) {
            sum += in;
            ++n;
            is_test = false;
            return sum / (T)n;
        }

        void update(const T in, T &out) {
            sum += in;
            ++n;
            out = sum / (T)n;
            is_test = false;
        }

        double test(const T in) {
            T _sum = sum;
            tn = n;
            _sum += in;
            ++tn;
            is_test = true;
            return _sum / (T)tn;
        }

        void test(const T in, T &out) {
            T _sum = sum;
            tn = n;
            _sum += in;
            ++tn;
            out = _sum / (T)tn;
            is_test = true;
        }

        int get_period() {return is_test ? tn : n;};

        void clear() {
            n = 0;
            sum = 0;
        }
    };

    /** \brief Кумулятивное скользящее среднее с объемом
     */
    template <typename T>
    class VCMA {
    private:
        uint64_t n = 0;
        uint64_t tn = 0;
        T sum = 0;
        T sum_weight = 0;
        bool is_test = false;
    public:

        VCMA() {};

        double update(const T input, const T weight) {
            sum += input * weight;
            sum_weight += weight;
            ++n;
            is_test = false;
            if(sum_weight == 0) return 0.0;
            return sum / sum_weight;
        }

        void update(const T input, const T weight, T &out) {
            sum += input * weight;
            sum_weight += weight;
            ++n;
            if(sum_weight == 0) out = 0.0;
            else out = sum / sum_weight;
            is_test = false;
        }

        double test(const T input, const T weight) {
            tn = n;
            T _sum = sum;
            T _sum_weight = sum_weight;

            _sum += input * weight;
            _sum_weight += weight;
            ++tn;
            is_test = true;
            if(_sum_weight == 0) return 0.0;
            return _sum / _sum_weight;
        }

        void test(const T input, const T weight, T &out) {
            tn = n;
            T _sum = sum;
            T _sum_weight = sum_weight;

            _sum += input * weight;
            _sum_weight += weight;
            ++tn;
            if(_sum_weight == 0) out = 0.0;
            else out = _sum / _sum_weight;
            is_test = true;
        }

        int get_period() {return is_test ? tn : n;};

        void clear() {
            n = 0;
            tn = 0;
            sum = 0;
            sum_weight = 0;
            is_test = false;
        }
    };

    /** \brief Скользящая сумма
     */
    template <typename T>
    class SUM {
    private:
        std::vector<T> data_;
        size_t period_ = 0;
    public:
        SUM() {};
        /** \brief Инициализировать скользящую сумму
         * \param period период
         */
        SUM(const size_t period) : period_(period) {
            data_.reserve(period_);
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in, T &out) {
            if(period_ == 0) {
                out = in;
                return NO_INIT;
            }
            if(data_.size() < (size_t)period_) {
                data_.push_back(in);
                if(data_.size() == (size_t)period_) {
                    out = std::accumulate(data_.begin(), data_.end(), (T)0);
                    return OK;
                }
            } else {
                data_.push_back(in);
                data_.erase(data_.begin());
                out = std::accumulate(data_.begin(), data_.end(), (T)0);
                return OK;
            }
            out = in;
            return INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Протестировать индикатор
         *
         * Данный метод отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in, T &out) {
            if(period_ == 0) {
                out = in;
                return NO_INIT;
            }
            std::vector<T> _data = data_;
            if(_data.size() < (size_t)period_) {
                _data.push_back(in);
                if(_data.size() == (size_t)period_) {
                    out = std::accumulate(_data.begin(), _data.end(), (T)0);
                    return OK;
                }
            } else {
                _data.push_back(in);
                _data.erase(_data.begin());
                out = std::accumulate(_data.begin(), _data.end(), (T)0);
                return OK;
            }
            out = in;
            return INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Очистить данные индикатора
         */
        void clear() {
            data_.clear();
        }
    };

    /** \brief Взвешенное скользящее среднее
     */
    template <typename T>
    class WMA {
    private:
        std::vector<T> data_;
        size_t period_ = 0;
    public:
        WMA() {};
        /** \brief Инициализировать взвешенное скользящее среднее
         * \param period период
         */
        WMA(const size_t period) : period_(period) {
            data_.reserve(period_);
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in, T &out) {
            if(period_ == 0) {
                out = in;
                return NO_INIT;
            }
            if(data_.size() < (size_t)period_) {
                data_.push_back(in);
                if(data_.size() == (size_t)period_) {
                    T sum = 0;
                    for(size_t i = data_.size(); i > 0; i--) {
                        sum += data_[i - 1] * (T)i;
                    }
                    out = (sum * 2.0d) / ((T)period_ * ((T)period_ + 1.0d));
                    return OK;
                }
            } else {
                data_.push_back(in);
                data_.erase(data_.begin());
                T sum = 0;
                for(size_t i = data_.size(); i > 0; i--) {
                    sum += data_[i - 1] * (T)i;
                }
                out = (sum * 2.0d) / ((T)period_ * ((T)period_ + 1.0d));
                return OK;
            }
            out = in;
            return INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Протестировать индикатор
         *
         * Данный метод отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in, T &out) {
            if(period_ == 0) {
                out = in;
                return NO_INIT;
            }
            std::vector<T> _data = data_;
            if(_data.size() < (size_t)period_) {
                _data.push_back(in);
                if(_data.size() == (size_t)period_) {
                    T sum = 0;
                    for(size_t i = _data.size(); i > 0; i--) {
                        sum += _data[i - 1] * (T)i;
                    }
                    out = (sum * 2.0d) / ((T)period_ * ((T)period_ + 1.0d));
                    return OK;
                }
            } else {
                _data.push_back(in);
                _data.erase(_data.begin());
                T sum = 0;
                for(size_t i = data_.size(); i > 0; i--) {
                    sum += data_[i - 1] * (T)i;
                }
                out = (sum * 2.0d) / ((T)period_ * ((T)period_ + 1.0d));
                return OK;
            }
            out = in;
            return INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Очистить данные индикатора
         */
        void clear() {
            data_.clear();
        }
    };

    /** \brief Экспоненциально взвешенное скользящее среднее
     */
    template <class T>
    class EMA {
    protected:
        std::vector<T> data_;
        T last_data_;
        T a;
        size_t period_ = 0;
    public:
        EMA() {};

        /** \brief Инициализировать экспоненциально взвешенное
         * скользящее среднее
         * \param period период
         */
        EMA(const size_t period) : period_(period) {
            data_.reserve(period_);
            a = 2.0/(T)(period_ + 1.0d);
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        virtual int update(const T in, T &out) {
            if(period_ == 0) {
                out = in;
                return NO_INIT;
            }
            if(data_.size() < (size_t)period_) {
                data_.push_back(in);
                if(data_.size() == (size_t)period_) {
                    T sum = std::accumulate(data_.begin(), data_.end(), T(0));
                    last_data_ = sum / (T)period_;
                }
            } else {
                last_data_ = a * in + (1.0 - a) * last_data_;
                out = last_data_;
                return OK;
            }
            out = in;
            return INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем, что не влияет на внутреннее
         * состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T &in, T &out) {
            if(period_ == 0) {
                out = in;
                return NO_INIT;
            }
            if(data_.size() == period_) {
                out = a * in + (1.0 - a) * last_data_;
                return OK;
            }
            out = in;
            return INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Очистить данные индикатора
         */
        void clear() {
            data_.clear();
        }
    };

    /** \brief Модифицированное скользящее среднее
     */
    template <class T>
    class MMA : public EMA<T> {
    public:
        MMA() {};

        /** \brief Инициализировать модифицированное скользящее среднее
         * \param period период
         */
        MMA(const size_t period) {
            EMA<T>::period_ = period;
            EMA<T>::data_.reserve(period);
            EMA<T>::a = 1.0/(T)period;
        }
    };

    /** \brief Индикатор Volume Weighted MA - модифицированная скользящая средняя, с реализацией взвешенности по объему.
     */
    template <typename T>
    class VWMA {
    private:
        std::vector<T> price_data;
        std::vector<T> weight_data;
        size_t period = 0;
    public:
        VWMA() {};

        /** \brief Инициализировать скользящее среднее
         * \param period период
         */
        VWMA(const size_t user_period) : period(user_period) {
            price_data.reserve(period);
            weight_data.reserve(period);
        }

        /** \brief Обновить состояние индикатора
         * \param input Сигнал на входе
         * \param weight Вес сигнала на входе
         * \param output Сигнал на выходе
         * \return Вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T input, const T weight, T &output) {
            if(period == 0) {
                output = input;
                return NO_INIT;
            }
            price_data.push_back(input);
            weight_data.push_back(weight);
            if(price_data.size() > period) {
                price_data.erase(price_data.begin());
                weight_data.erase(weight_data.begin());
            }
            if(price_data.size() == period) {
                T sum = 0;
                T sum_weight = 0;
                for(size_t i = 0; i < price_data.size(); ++i) {
                    sum += price_data[i] * weight_data[i];
                    sum_weight += weight_data[i];
                }
                if(sum_weight == 0) {
                    sum = 0;
                    for(size_t i = 0; i < price_data.size(); ++i) {
                        sum += price_data[i];
                    }
                    output = sum / (T)period;
                } else {
                    output = sum / (sum_weight);
                }
            } else {
                output = input;
                return INDICATOR_NOT_READY_TO_WORK;
            }
            return OK;
        }

        /** \brief Протестировать индикатор
         *
         * Данный метод отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
         int test(const T input, const T weight, T &output) {
            if(period == 0) {
                output = input;
                return NO_INIT;
            }
            std::vector<T> _price_data = price_data;
            std::vector<T> _weight_data = weight_data;

            _price_data.push_back(input);
            _weight_data.push_back(weight);
            if(_price_data.size() > period) {
                _price_data.erase(price_data.begin());
                _weight_data.erase(weight_data.begin());
            }
            if(_price_data.size() == period) {
                T sum = 0;
                T sum_weight = 0;
                for(size_t i = 0; i < _price_data.size(); ++i) {
                    sum += _price_data[i] * _weight_data[i];
                    sum_weight += _weight_data[i];
                }
                if(sum_weight == 0) {
                    sum = 0;
                    for(size_t i = 0; i < _price_data.size(); ++i) {
                        sum += _price_data[i];
                    }
                    output = sum / (T)period;
                } else {
                    output = sum / sum_weight;
                }
            } else {
                output = input;
                return INDICATOR_NOT_READY_TO_WORK;
            }
            return OK;
        }

        /** \brief Очистить данные индикатора
         */
        void clear() {
            price_data.clear();
            weight_data.clear();
        }
    };

    /** \brief Скользящее окно
     */
    template <typename T>
    class MW {
    private:
        std::vector<T> data_;
        std::vector<T> data_test_;
        size_t period_ = 0;
        bool is_test_ = false;
    public:
        MW() {};

        /** \brief Инициализировать скользящее окно
         * \param period период
         */
        MW(const size_t period) : period_(period) {
            data_.reserve(period_);
        }

        /** \brief Проверить инициализацию буфера скользящего окна
         * \return Вернет true, если буфер скользящего окна
         * полностью заполнен значениями.
         */
        bool is_init() {
            return (data_.size() == period_);
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \param out массив на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &in, std::vector<T> &out) {
            is_test_ = false;
            if(period_ == 0) return NO_INIT;
            if(data_.size() < period_) {
                data_.push_back(in);
                if(data_.size() == period_) {
                    out = data_;
                    return OK;
                }
            } else {
                data_.push_back(in);
                data_.erase(data_.begin());
                out = data_;
                return OK;
            }
            return INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &in) {
            is_test_ = false;
            if(period_ == 0) return NO_INIT;
            if(data_.size() < period_) {
                data_.push_back(in);
                if(data_.size() == period_) {
                    return OK;
                }
            } else {
                data_.push_back(in);
                data_.erase(data_.begin());
                return OK;
            }
            return INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем, что не влияет на внутреннее
         * состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T &in, std::vector<T> &out) {
            is_test_ = true;
            if(period_ == 0) return NO_INIT;
            data_test_ = data_;
            if(data_test_.size() < period_) {
                data_test_.push_back(in);
                if(data_test_.size() == period_) {
                    out = data_test_;
                    return OK;
                }
            } else {
                data_test_.push_back(in);
                data_test_.erase(data_test_.begin());
                out = data_test_;
                return OK;
            }
            return INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем, что не влияет на внутреннее
         * состояние индикатора
         * \param in сигнал на входе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T &in) {
            is_test_ = true;
            if(period_ == 0) return NO_INIT;
            data_test_ = data_;
            if(data_test_.size() < period_) {
                data_test_.push_back(in);
                if(data_test_.size() == period_) {
                    return OK;
                }
            } else {
                data_test_.push_back(in);
                data_test_.erase(data_test_.begin());
                return OK;
            }
            return INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Получить данные внутреннего буфера индикатора
         * \param buffer буфер
         */
        void get_data(std::vector<T> &buffer) {
            if(is_test_) buffer = data_test_;
            else buffer = data_;
        }

        /** \brief Получить максимальное значение буфера
         * \param max_value Максимальное значение
         * \param period Период максимальных данных
         * \param offset Смещение в массиве. По умолчанию 0
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int get_max_value(
                T &max_value,
                const size_t period,
                const size_t offset = 0) {
            const size_t total_offset = period + offset;
            if(is_test_ && data_test_.size() < total_offset)
                return xtechnical_common::INVALID_PARAMETER;
            else if(!is_test_ && data_.size() < total_offset)
                return xtechnical_common::INVALID_PARAMETER;

            if(is_test_) max_value = *std::max_element(
                data_test_.end() - total_offset,
                data_test_.end() - offset);
            else max_value = *std::max_element(
                data_.end() - total_offset,
                data_.end() - offset);
            return xtechnical_common::OK;
        }

        /** \brief Получить минимальное значение буфера
         * \param min_value Минимальное значение
         * \param period Период минимальных данных
         * \param offset Смещение в массиве. По умолчанию 0
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int get_min_value(
                T &min_value,
                const size_t period,
                const size_t offset = 0) {
            const size_t total_offset = period + offset;
            if(is_test_ && data_test_.size() < total_offset)
                return xtechnical_common::INVALID_PARAMETER;
            else if(!is_test_ && data_.size() < total_offset)
                return xtechnical_common::INVALID_PARAMETER;

            if(is_test_) min_value = *std::min_element(
                data_test_.end() - total_offset,
                data_test_.end() - offset);
            else min_value = *std::min_element(
                data_.end() - total_offset,
                data_.end() - offset);
            return xtechnical_common::OK;
        }

        /** \brief Получить сумму
         * \param sum_value Сумма
         * \param period Период минимальных данных
         * \param offset Смещение в массиве. По умолчанию 0
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int get_sum(T &sum_value,
                const size_t period,
                const size_t offset = 0) {
            const size_t total_offset = period + offset;
            if(is_test_ && data_test_.size() < total_offset)
                return xtechnical_common::INVALID_PARAMETER;
            else if(!is_test_ && data_.size() < total_offset)
                return xtechnical_common::INVALID_PARAMETER;

            if(is_test_) sum_value = std::accumulate(
                data_test_.end() - total_offset,
                data_test_.end() - offset,
                (T)0);
            else sum_value = std::accumulate(
                data_.end() - total_offset,
                data_.end() - offset,
                (T)0);
            return xtechnical_common::OK;
        }

        /** \brief Получить нормализованные данные
         * \param buffer буфер
         * \param type Тип нормализации.
         * На выбор: MINMAX_UNSIGNED, MINMAX_SIGNED, Z_SCORE_TRANSFORMING
         * \param period Период минимальных данных
         * \param offset Смещение в массиве. По умолчанию 0
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int get_normalized_data(
                std::vector<T> &buffer,
                const uint32_t type,
                const size_t period,
                const size_t offset = 0) {
            const size_t total_offset = period + offset;
            if(is_test_ && data_test_.size() < total_offset)
                return xtechnical_common::INVALID_PARAMETER;
            else if(!is_test_ && data_.size() < total_offset)
                return xtechnical_common::INVALID_PARAMETER;

            std::vector<T> fragment;
            if(is_test_) fragment.assign(
                data_test_.end() - total_offset,
                data_test_.end() - offset);
            else fragment.assign(
                data_.end() - total_offset,
                data_.end() - offset);
            if( type == xtechnical_common::MINMAX_UNSIGNED ||
                type == xtechnical_common::MINMAX_UNSIGNED) {
                buffer.resize(fragment.size());
                xtechnical_normalization::calculate_min_max(
                    fragment,
                    buffer,
                    type);
            } else
            if(type == xtechnical_common::Z_SCORE_TRANSFORMING) {
                buffer.resize(fragment.size());
                xtechnical_normalization::calculate_zscore(fragment, buffer);
            }

            return xtechnical_common::OK;
        }

        /** \brief Получить нормализованные данные
         * \param buffer Буфер
         * \param type Тип нормализации.
         * На выбор: MINMAX_UNSIGNED, MINMAX_SIGNED, Z_SCORE_TRANSFORMING
         * \param min_level Заданный минимальный уровень
         * \param max_level Заданный максимальный уровень
         * \param period Период минимальных данных
         * \param offset Смещение в массиве. По умолчанию 0
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int get_normalized_data(
                std::vector<T> &buffer,
                const uint32_t type,
                const T min_level,
                const T max_level,
                const size_t period,
                const size_t offset = 0) {
            const size_t total_offset = period + offset;
            if(is_test_ && data_test_.size() < total_offset)
                return xtechnical_common::INVALID_PARAMETER;
            else if(!is_test_ && data_.size() < total_offset)
                return xtechnical_common::INVALID_PARAMETER;

            std::vector<T> fragment;
            if(is_test_) fragment.assign(
                data_test_.end() - total_offset,
                data_test_.end() - offset);
            else fragment.assign(
                data_.end() - total_offset,
                data_.end() - offset);
            if(type == xtechnical_common::MINMAX_UNSIGNED ||
                type == xtechnical_common::MINMAX_SIGNED) {
                buffer.resize(fragment.size());
                xtechnical_normalization::calculate_min_max(
                    fragment,
                    buffer,
                    min_level,
                    max_level,
                    type);
            } else
            if(type == xtechnical_common::Z_SCORE_TRANSFORMING) {
                buffer.resize(fragment.size());
                xtechnical_normalization::calculate_zscore(fragment, buffer);
            }

            return xtechnical_common::OK;
        }

        /** \brief Экспериментальный метод
         */
        int compare_data(
                T &compare_result,
                const uint32_t compare_type,
                const bool is_use_min_max_level,
                const T min_level,
                const T max_level,
                const size_t period,
                const size_t offset = 0) {
            std::vector<T> fragment;
            int err = xtechnical_common::OK;
            if(!is_use_min_max_level) err = get_normalized_data(
                fragment,
                MINMAX_SIGNED,
                period,
                offset);
            else err = get_normalized_data(
                fragment,
                MINMAX_SIGNED,
                min_level,
                max_level,
                period,
                offset);
            if(err != xtechnical_common::OK) return err;
            if(compare_type == xtechnical_common::COMPARE_WITH_ZERO_LINE) {
                compare_result = 0;
                for(size_t i = 0; i < period; ++i) {
                    compare_result += std::abs(fragment[i]);
                }
                compare_result /= (T)period;
                compare_result = 1.0 - compare_result;
            } else
            if(compare_type == xtechnical_common::COMPARE_WITH_STRAIGHT_LINE) {
                /* сначала найдем минимум или максимум */
                T new_max_level = -1.1;
                T new_min_level = 1.1;
                for(size_t i = 0; i < period; ++i) {
                    if(fragment[i] < new_min_level) new_min_level = fragment[i];
                    if(fragment[i] > new_max_level) new_max_level = fragment[i];
                }

                /* находим два значения */
                T result_min = 0;
                T result_max = 0;
                for(size_t i = 0; i < period; ++i) {
                    result_min += std::abs(fragment[i] - new_min_level);
                    result_max += std::abs(new_max_level - fragment[i]);
                }
                result_min /= (T)period;
                result_max /= (T)period;
                compare_result = std::max(result_min, result_max);
                compare_result = 1.0 - compare_result;
            } else
            if(compare_type == xtechnical_common::COMPARE_WITH_CENTER_LINE) {
                /* сначала найдем минимум или максимум */
                T new_max_level = -1.1;
                T new_min_level = 1.1;
                for(size_t i = 0; i < period; ++i) {
                    if(fragment[i] < new_min_level) new_min_level = fragment[i];
                    if(fragment[i] > new_max_level) new_max_level = fragment[i];
                }

                /* находим два значения */
                compare_result = 0;
                T center_line = (new_max_level - new_min_level) / 2.0;
                for(size_t i = 0; i < period; ++i) {
                    compare_result += std::abs(fragment[i] - center_line);
                }
                compare_result /= (T)period;
                compare_result = 1.0 - compare_result;
            } else
            if(compare_type == xtechnical_common::CALCULATE_ANGLE) {
                /* найдем приращение */
                T average_increase = 0;
                for(size_t i = 1; i < period; ++i) {
                    average_increase += (fragment[i] - fragment[i - 1]);
                }
                average_increase /= (T)(period - 1);
                /* найдем угол */
                compare_result = std::atan(-average_increase / -1.0);
            }
            return xtechnical_common::OK;
        }

        /** \brief Получить среднее значение буфера
         * \param average_value Среднее значение
         * \param period Период среднего значения
         * \param offset Смещение в массиве. По умолчанию 0
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int get_average(
                T &average_value,
                const size_t period,
                const size_t offset = 0) {
            const size_t total_offset = period + offset;
            if(is_test_ && data_test_.size() < total_offset)
                return xtechnical_common::INVALID_PARAMETER;
            else if(!is_test_ && data_.size() < total_offset)
                return xtechnical_common::INVALID_PARAMETER;

            if(is_test_) {
                T sum = std::accumulate(
                    data_test_.end() - period - offset,
                    data_test_.end() - offset,
                    T(0));
                average_value = sum / (T)period;
            } else {
                T sum = std::accumulate(
                    data_.end() - period - offset,
                    data_.end() - offset,
                    T(0));
                average_value = sum / (T)period;
            }
            return xtechnical_common::OK;
        }

        /** \brief Получить стандартное отклонение буфера
         * \param std_dev_value Стандартное отклонение
         * \param period Период стандартного отклонения
         * \param offset Смещение в массиве. По умолчанию 0
         */
        int get_std_dev(
                T &std_dev_value,
                const size_t period,
                const size_t offset = 0) {
            const size_t total_offset = period + offset;
            if(is_test_ && data_test_.size() < total_offset)
                return xtechnical_common::INVALID_PARAMETER;
            else if(!is_test_ && data_.size() < total_offset)
                return xtechnical_common::INVALID_PARAMETER;

            if(is_test_) {
                T ml = std::accumulate(
                    data_test_.end() - total_offset,
                    data_test_.end() - offset,
                    T(0));
                ml /= (T)period;
                T sum = 0;
                const size_t stop = data_test_.size() - offset;
                const size_t start = stop - period;
                for (size_t i = start; i < stop; ++i) {
                    T diff = (data_test_[i] - ml);
                    sum +=  diff * diff;
                }
                std_dev_value = std::sqrt(sum / (T)(period - 1));
            } else {
                T ml = std::accumulate(
                    data_.end() - total_offset,
                    data_.end() - offset,
                    T(0));
                ml /= (T)period;
                T sum = 0;
                const size_t stop = data_.size() - offset;
                const size_t start = stop - period;
                for (size_t i = start; i < stop; ++i) {
                    T diff = (data_[i] - ml);
                    sum +=  diff * diff;
                }
                std_dev_value = std::sqrt(sum / (T)(period - 1));
            }
            return xtechnical_common::OK;
        }

        /** \brief Получить массив средних значений
         * и стандартного отклонения буфера
         *
         * Минимальный период равен 2
         * \param average_data массив средних значений
         * \param std_data массив стандартного отклонения
         * \param min_period минимальный период
         * \param max_period максимальный период
         * \param step_period шаг периода
         */
        void get_average_and_std_dev_array(
                std::vector<T> &average_data,
                std::vector<T> &std_data,
                size_t min_period,
                size_t max_period,
                const size_t &step_period) {
            size_t reserve_size = 1 + (max_period - min_period)/step_period;
            --min_period;
            --max_period;
            average_data.clear();
            average_data.reserve(reserve_size);
            std_data.clear();
            std_data.reserve(reserve_size);
            if(is_test_) {
                T sum = 0;
                size_t num_element = 0;
                size_t data_size = data_.size();
                // начинаем список с конца
                for(int i = data_size - 1; i >= 0; --i) {
                    sum += data_test_[i]; // находим сумму элементов
                    if(num_element > max_period) break;
                    if(num_element >= min_period) {
                        ++num_element; // находим число элементов
                        T ml = (T)(sum/(T)num_element); // находим среднее
                        average_data.push_back(ml); // добавляем среднее
                        T sum_std = 0;
                        size_t max_len = data_size - num_element;
                        for(size_t j = data_size - 1; j >= max_len; --j) {
                            T diff = (data_test_[j] - ml);
                            sum_std += diff * diff;
                        }
                        std_data.push_back((T)std::sqrt(sum_std /
                            (T)(num_element - 1)));
                        min_period += step_period;
                    } else {
                        ++num_element;
                    }
                } // for i
            } else {
                T sum = 0;
                size_t num_element = 0;
                size_t data_size = data_.size();
                // начинаем список с конца
                for(int i = data_size - 1; i >= 0; --i) {
                    sum += data_[i]; // находим сумму элементов
                    if(num_element > max_period) break;
                    if(num_element >= min_period) {
                        ++num_element; // находим число элементов
                        T ml = (T)(sum/(T)num_element); // находим среднее
                        average_data.push_back(ml); // добавляем среднее
                        T sum_std = 0;
                        int max_len = data_size - num_element;
                        for(int j = data_size - 1; j >= max_len; j--) {
                            T diff = (data_[j] - ml);
                            sum_std += diff * diff;
                        }
                        std_data.push_back((T)std::sqrt(sum_std /
                            (T)(num_element - 1)));
                        min_period += step_period;
                    } else {
                        ++num_element;
                    }
                } // for i
            }
        }

        /** \brief Получить массив значений RSI
         * \param rsi_data массив значений RSI
         * \param min_period минимальный период
         * \param max_period максимальный период
         * \param step_period шаг периода
         */
        void get_rsi_array(
                std::vector<T> &rsi_data,
                size_t min_period,
                size_t max_period,
                const size_t &step_period) {
            size_t reserve_size = 1 + (max_period - min_period)/step_period;
            --min_period;
            --max_period;
            rsi_data.clear();
            rsi_data.reserve(reserve_size);
            if(is_test_) {
                T sum_u = 0, sum_d = 0;
                size_t num_element = 0;
                // начинаем список с конца
                for(size_t i = data_test_.size() - 1; i >= 1; --i) {
                    T u = 0, d = 0;
                    const T prev_ = data_test_[i - 1];
                    const T in_ = data_test_[i];
                    if(prev_ < in_) u = in_ - prev_;
                    else if(prev_ > in_) d = prev_ - in_;
                    sum_u += u;
                    sum_d += d;
                    if(num_element > max_period) break;
                    if(num_element >= min_period) {
                        ++num_element;
                        u = sum_u /(T)num_element;
                        d = sum_d /(T)num_element;
                        if(d == 0) rsi_data.push_back(100.0);
                        else rsi_data.push_back(((T)100.0 - ((T)100.0 /
                            ((T)1.0 + (u / d)))));
                        min_period += step_period;
                    } else {
                        ++num_element;
                    }
                } // for i
            } else {
                T sum_u = 0;
                T sum_d = 0;
                size_t num_element = 0;
                // начинаем список с конца
                for(size_t i = data_.size() - 1; i >= 1; --i) {
                    if(data_[i - 1] < data_[i])
                        sum_u += data_[i] - data_[i - 1];
                    else if(data_[i - 1] > data_[i])
                        sum_d += data_[i - 1] - data_[i];
                    if(num_element > max_period) break;
                    if(num_element >= min_period) {
                        ++num_element;
                        T u = sum_u /(T)num_element;
                        T d = sum_d /(T)num_element;
                        if(d == 0) rsi_data.push_back(100.0);
                        else rsi_data.push_back(((T)100.0 - ((T)100.0 /
                            ((T)1.0 + (u / d)))));
                        min_period += step_period;
                    } else {
                        ++num_element;
                    }
                } // for i
            }
        }

        /** \brief Получить значение RSI
         * \param rsi_value Значение RSI
         * \param period Период RSI
         */
        void get_rsi(T &rsi_value, const size_t period) {
            rsi_value = 50;
            if(is_test_) {
                T sum_u = 0, sum_d = 0;
                const size_t start_ind = data_test_.size() - 1;
                const size_t stop_ind = data_test_.size() - period;
                // начинаем список с конца
                for(size_t i = start_ind; i >= stop_ind; --i) {
                    T u = 0, d = 0;
                    const T prev_ = data_test_[i - 1];
                    const T in_ = data_test_[i];
                    if(prev_ < in_) u = in_ - prev_;
                    else if(prev_ > in_) d = prev_ - in_;
                    sum_u += u;
                    sum_d += d;
                } // for i
                T u = sum_u /(T)period;
                T d = sum_d /(T)period;
                if(d == 0) rsi_value = 100.0;
                else rsi_value = ((T)100.0 - ((T)100.0 / ((T)1.0 + (u / d))));
            } else {
                T sum_u = 0;
                T sum_d = 0;
                const size_t start_ind = data_.size() - 1;
                const size_t stop_ind = data_.size() - period;
                // начинаем список с конца
                for(size_t i = start_ind; i >= stop_ind; --i) {
                    if(data_[i - 1] < data_[i])
                        sum_u += data_[i] - data_[i - 1];
                    else if(data_[i - 1] > data_[i])
                        sum_d += data_[i - 1] - data_[i];
                } // for i
                T u = sum_u /(T)period;
                T d = sum_d /(T)period;
                if(d == 0) rsi_value = 100.0;
                else rsi_value = ((T)100.0 - ((T)100.0 / ((T)1.0 + (u / d))));
            }
        }

        /** \brief Очистить данные индикатора
         */
        void clear() {
            data_.clear();
            data_test_.clear();
        }
    };

    /** \brief Класс фильтра низкой частоты
     */
    template <typename T>
    class LowPassFilter {
    private:
        T alfa_ = 0;
        T beta_ = 0;
        T prev_ = 0;
        T tranTime = 0;
        bool is_update_ = false;
        bool is_init_ = false;
    public:

        LowPassFilter() {};

        /** \brief Инициализация фильтра низкой частоты
         * \param dt время переходного процесса
         * \param period период дискретизации
         * \param error_signal ошибка сигнала
         */
        LowPassFilter(
                const T dt,
                const T period = 1.0,
                const T error_signal = 0.03) {
            T N = dt / period;
            T Ntay = std::log(1.0 / error_signal);
            alfa_ = std::exp(-Ntay / N);
            beta_ = 1.0 - alfa_;
            is_init_ = true;
        }

        /** \brief Получить новые данные индикатора
         * \param in сигнал на входе
         * \param out массив на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &in, T &out) {
            if(!is_init_) return NO_INIT;
            if (!is_update_) {
                prev_ = in;
                is_update_ = true;
                out = in;
                return INDICATOR_NOT_READY_TO_WORK;
            }
            out = alfa_ * prev_ + beta_ * in;
            prev_ = out;
            return OK;
        }

        /** \brief Получить новые данные индикатора
         * \param in сигнал на входе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &in) {
            if(!is_init_) return NO_INIT;
            if (!is_update_) {
                prev_ = in;
                is_update_ = true;
                return INDICATOR_NOT_READY_TO_WORK;
            }
            prev_ = alfa_ * prev_ + beta_ * in;
            return OK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in, T &out) {
            if(!is_init_) {
                return NO_INIT;
            }
            if (!is_init_) {
                out = in;
                return INDICATOR_NOT_READY_TO_WORK;
            }
            out = alfa_ * prev_ + beta_ * in;
            return OK;
        }

        /** \brief Очистить данные индикатора
         */
        void clear() {
            is_update_ = false;
        }
    };

    /** \brief Индекс относительной силы
     */
    template <typename T, class INDICATOR_TYPE>
    class RSI {
    private:
        INDICATOR_TYPE iU;
        INDICATOR_TYPE iD;
        bool is_init_ = false;
        bool is_update_ = false;
        T prev_ = 0;
    public:
        RSI() {}

        /** \brief Инициализировать индикатор индекса относительной силы
         * \param period период индикатора
         */
        RSI(const size_t &period) : iU(period), iD(period) {
            is_init_ = true;
        }

        /** \brief Инициализировать индикатор индекса относительной силы
         * \param period период индикатора
         */
        void init(const size_t &period) {
            is_init_ = true;
            is_update_ = false;
            iU = INDICATOR_TYPE(period);
            iD = INDICATOR_TYPE(period);
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &in, T &out) {
            if(!is_init_) {
                return NO_INIT;
            }
            if(!is_update_) {
                prev_ = in;
                is_update_ = true;
                out = 50.0;
                return INDICATOR_NOT_READY_TO_WORK;
            }
            T u = 0;
            T d = 0;
            if(prev_ < in) {
                u = in - prev_;
            } else
            if(prev_ > in) {
                d = prev_ - in;
            }
            int erru, errd = 0;
            T mu = 0;
            T md = 0;
            erru = iU.update(u, mu);
            errd = iD.update(d, md);
            prev_ = in;
            if(erru != OK || errd != OK) {
                out = 50.0;
                return INDICATOR_NOT_READY_TO_WORK;
            }
            if(md == 0) {
                out = 100.0;
                return OK;
            }
            T rs = mu / md;
            out = 100.0 - (100.0 / (1.0 + rs));
            return OK;
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &in) {
            if(!is_init_) {
                return NO_INIT;
            }
            if(!is_update_) {
                prev_ = in;
                is_update_ = true;
                return INDICATOR_NOT_READY_TO_WORK;
            }
            T u = 0, d = 0;
            if(prev_ < in) {
                u = in - prev_;
            } else
            if(prev_ > in) {
                d = prev_ - in;
            }
            int erru, errd = 0;
            erru = iU.update(u, u);
            errd = iD.update(d, d);
            prev_ = in;
            if(erru != OK || errd != OK) {
                return INDICATOR_NOT_READY_TO_WORK;
            }
            return OK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T &in, T &out) {
            if(!is_init_) {
                return NO_INIT;
            }
            if(!is_update_) {
                out = 50.0;
                return INDICATOR_NOT_READY_TO_WORK;
            }
            T u = 0, d = 0;
            if(prev_ < in) {
                u = in - prev_;
            } else
            if(prev_ > in) {
                d = prev_ - in;
            }
            int erru, errd = 0;
            erru = iU.test(u, u);
            errd = iD.test(d, d);
            if(erru != OK || errd != OK) {
                out = 50.0;
                return INDICATOR_NOT_READY_TO_WORK;
            }
            if(d == 0) {
                out = 100.0;
                return OK;
            }
            T rs = u / d;
            out = 100.0 - (100.0 / (1.0 + rs));
            return OK;
        }

        /** \brief Очистить данные индикатора
         */
        void clear() {
            is_update_ = false;
            iU.clear();
            iD.clear();
        }
    };

    template <class T1, class T2>
    int calc_ring_rsi(const T1 &in, T2 &out, const size_t &period) {
        size_t input_size = in.size();
        size_t output_size = out.size();
        if( input_size == 0 || input_size < period ||
            output_size != input_size)
            return INVALID_PARAMETER;
        using NumType = typename T1::value_type;
        RSI<NumType,SMA<NumType>> iRSI(period);
        for(size_t i = input_size - period; i < input_size; ++i) {
            iRSI.update(in[i]);
        }
        for(size_t i = 0; i < input_size; ++i) {
            iRSI.update(in[i], out[i]);
        }
        return OK;
    }

    /** \brief Линии Боллинджера
     */
    template <typename T>
    class BollingerBands {
    private:
        std::vector<T> data_;
        size_t period_ = 0;
        T d_;
    public:
        BollingerBands() {};

        /** \brief Инициализация линий Боллинджера
         * \param period период  индикатора
         * \param factor множитель стандартного отклонения
         */
        BollingerBands(const size_t &period, const T &factor) {
            period_ = period;
            d_ = factor;
        }

        /** \brief Инициализация линий Боллинджера
         * \param period период  индикатора
         * \param factor множитель стандартного отклонения
         */
        void init(const size_t &period, const T &factor) {
            period_ = period;
            d_ = factor;
            data_.clear();
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \param tl верхняя полоса боллинджера
         * \param ml среняя полоса боллинджера
         * \param bl нижняя полоса боллинджера
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &in, T &tl, T &ml, T &bl) {
            if(period_ == 0) {
                tl = in;
                ml = in;
                bl = in;
                return NO_INIT;
            }
            if(data_.size() < (size_t)period_) {
                data_.push_back(in);
                if(data_.size() != (size_t)period_) {
                    tl = in;
                    ml = in;
                    bl = in;
                    return INDICATOR_NOT_READY_TO_WORK;
                }
            } else {
                data_.push_back(in);
                data_.erase(data_.begin());
            }
            ml = std::accumulate(data_.begin(), data_.end(), T(0));
            ml /= (T)period_;
            T sum = 0;
            for (size_t i = 0; i < period_; ++i) {
                T diff = (data_[i] - ml);
                sum +=  diff * diff;
            }
            T std_dev = std::sqrt(sum / (T)(period_ - 1));
            tl = std_dev * d_ + ml;
            bl = ml - std_dev * d_;
            return OK;
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \param ml среняя полоса боллинджера
         * \param std_dev стандартное отклонение
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &in, T &ml, T &std_dev) {
            if(period_ == 0) {
                ml = 0;
                std_dev = 0;
                return NO_INIT;
            }
            if(data_.size() < period_) {
                data_.push_back(in);
                if(data_.size() != period_) {
                    ml = 0;
                    std_dev = 0;
                    return INDICATOR_NOT_READY_TO_WORK;
                }
            } else {
                data_.push_back(in);
                data_.erase(data_.begin());
            }
            ml = std::accumulate(data_.begin(), data_.end(), T(0));
            ml /= (T)period_;
            T sum = 0;
            for (size_t i = 0; i < period_; ++i) {
                T diff = (data_[i] - ml);
                sum +=  diff * diff;
            }
            std_dev = std::sqrt(sum / (T)(period_ - 1));
            return OK;
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &in) {
            if(period_ == 0) {
                return NO_INIT;
            }
            if(data_.size() < period_) {
                data_.push_back(in);
                if(data_.size() != period_) {
                    return INDICATOR_NOT_READY_TO_WORK;
                }
            } else {
                data_.push_back(in);
                data_.erase(data_.begin());
            }
            return OK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in сигнал на входе
         * \param tl верхняя полоса боллинджера
         * \param ml среняя полоса боллинджера
         * \param bl нижняя полоса боллинджера
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T &in, T &tl, T &ml, T &bl) {
            if(period_ == 0) {
                tl = in;
                ml = in;
                bl = in;
                return NO_INIT;
            }
            std::vector<T> data_test = data_;
            if(data_test.size() < period_) {
                data_test.push_back(in);
                if(data_test.size() != period_) {
                    tl = in;
                    ml = in;
                    bl = in;
                    return INDICATOR_NOT_READY_TO_WORK;
                }
            } else {
                data_test.push_back(in);
                data_test.erase(data_test.begin());
            }
            ml = std::accumulate(data_test.begin(), data_test.end(), T(0));
            ml /= (T)period_;
            T sum = 0;
            for (size_t i = 0; i < period_; ++i) {
                T diff = (data_test[i] - ml);
                sum +=  diff * diff;
            }
            T std_dev = std::sqrt(sum / (T)(period_ - 1));
            tl = std_dev * d_ + ml;
            bl = ml - std_dev * d_;
            return OK;
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in сигнал на входе
         * \param ml среняя полоса боллинджера
         * \param std_dev стандартное отклонение
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T &in, T &ml, T &std_dev) {
            if(period_ == 0) {
                ml = in;
                std_dev = 0;
                return NO_INIT;
            }
            std::vector<T> data_test = data_;
            if(data_test.size() < period_) {
                data_test.push_back(in);
                if(data_test.size() != period_) {
                    ml = 0;
                    std_dev = 0;
                    return INDICATOR_NOT_READY_TO_WORK;
                }
            } else {
                data_test.push_back(in);
                data_test.erase(data_test.begin());
            }
            ml = std::accumulate(data_test.begin(), data_test.end(), T(0));
            ml /= (T)period_;
            T sum = 0;
            for (size_t i = 0; i < period_; ++i) {
                T diff = (data_test[i] - ml);
                sum +=  diff * diff;
            }
            std_dev = std::sqrt(sum / (T)(period_ - 1));
            return OK;
        }

        /** \brief Очистить данные индикатора
         */
        void clear() {
            data_.clear();
        }
    };

    /** \brief Обработать массив данных боллинджером по кругу
     */
    template <class T1, class T2>
    int calc_ring_bollinger(
            T1 &in,
            T2 &tl,
            T2 &ml,
            T2 &bl,
            const size_t &period,
            const double &std_dev_factor) {
        size_t input_size = in.size();
        size_t tl_size = tl.size();
        if( input_size == 0 || input_size < period ||
            tl_size != bl.size() || tl_size != input_size ||
            ml.size() != input_size) return INVALID_PARAMETER;
        using NumType = typename T1::value_type;
        BollingerBands<NumType> iBB(period, std_dev_factor);
        for(size_t i = input_size - period; i < input_size; ++i) {
            iBB.update(in[i]);
        }
        for(size_t i = 0; i < input_size; ++i) {
            iBB.update(in[i], tl[i], ml[i], bl[i]);
        }
        return OK;
    }

    /** \brief Средняя скорость
     */
    template <typename T>
    class AverageSpeed {
    private:
        MW<T> iMW;
        bool is_init_ = false;
    public:
        AverageSpeed() {};

        /** \brief Инициализировать класс индикатора
         * \param period период индикатора
         */
        AverageSpeed(const size_t &period) : iMW(period + 1) {
            is_init_ = true;
        }

        /** \brief Обновить состояние индикатора
         * \param in Цена
         * \param out Значение индикатора
         * \return вернет 0 в случае успеха
         */
        int update(const T &in, T &out) {
            if(!is_init_) return NO_INIT;
            std::vector<T> mw_out;
            int err = iMW.update(in, mw_out);
            if(err == OK) {
                std::vector<T> mw_diff(mw_out.size());
                xtechnical_normalization::calculate_difference(mw_out, mw_diff);
                T sum = std::accumulate(mw_diff.begin(), mw_diff.end(), T(0));
                out = sum /(T)mw_diff.size();
                return OK;
            }
            return err;
        }

        /** \brief Протестировать индикатор
         * \param in Цена
         * \param out Значение индикатора
         * \return вернет 0 в случае успеха
         */
        int test(const T &in, T &out) {
            if(!is_init_) return NO_INIT;
            std::vector<T> mw_out;
            int err = iMW.test(in, mw_out);
            if(err == OK) {
                std::vector<T> mw_diff(mw_out.size());
                xtechnical_normalization::calculate_difference(mw_out, mw_diff);
                T sum = std::accumulate(mw_diff.begin(), mw_diff.end(), T(0));
                out = sum /(T)mw_diff.size();
                return OK;
            }
            return err;
        }

        /** \brief Очистить состояние индикатора
         */
        void clear() {
            iMW.clear();
        }
    };
//------------------------------------------------------------------------------
    /** \brief Экспериментальный индикатор, не применять!
     */
    template <typename T>
    class DetectorWaveform {
    private:
        MW<T> iMW;
        const size_t MIN_WAVEFORM_LEN = 3;
        T coeff_exp = 3.141592;
        std::vector<std::vector<T>> exp_data_up_;
        std::vector<std::vector<T>> exp_data_dn_;

        void init_exp_data_up(std::vector<T> &data) {
            T dt = 1.0/(T)data.size();
            for(size_t i = 0; i < data.size(); ++i) {
                data[i] = exp(coeff_exp*(T)i*dt);
            }
            xtechnical_normalization::calculate_min_max(
                data,
                data,
                xtechnical_normalization::MINMAX_UNSIGNED);
        }

        void init_exp_data_dn(std::vector<T> &data) {
            T dt = 1.0/(T)data.size();
            for(size_t i = 0; i < data.size(); ++i) {
                data[i] = -exp(coeff_exp*(T)i*dt);
            }
            xtechnical_normalization::calculate_min_max(
                data,
                data,
                xtechnical_normalization::MINMAX_UNSIGNED);
        }
    public:
        /** \brief Инициализировать класс
         * \param max_len максимальная длина файла
         */
        DetectorWaveform(const int max_len) : iMW(max_len) {
            if(max_len < MIN_WAVEFORM_LEN) return;
            size_t max_num_exp_data = max_len - MIN_WAVEFORM_LEN + 1;
            exp_data_up_.resize(max_num_exp_data);
            exp_data_dn_.resize(max_num_exp_data);
            for(size_t l = MIN_WAVEFORM_LEN; l <= max_len; ++l) {
                exp_data_up_[l-MIN_WAVEFORM_LEN].resize(l);
                exp_data_dn_[l-MIN_WAVEFORM_LEN].resize(l);
                init_exp_data_up(exp_data_up_[l-MIN_WAVEFORM_LEN]);
                init_exp_data_dn(exp_data_dn_[l-MIN_WAVEFORM_LEN]);
            }
        }

        int update(T in, T &out, const int len_waveform) {
            std::vector<T> mw_out;
            int err = iMW.update(in, mw_out);
            if(err == OK) {
                if(mw_out.size() >= MIN_WAVEFORM_LEN &&
                    len_waveform <= mw_out.size()) {
                    std::vector<T> fragment_data;
                    fragment_data.insert(
                        fragment_data.begin(),
                        mw_out.begin() + mw_out.size() - len_waveform,
                        mw_out.end());
                    int err_n = xtechnical_normalization::calculate_min_max(
                        fragment_data,
                        fragment_data,
                        MINMAX_UNSIGNED);
                    if(err_n != OK) return err_n;
                    T coeff_up = 0, coeff_dn = 0;
                    int err_up = xtechnical_correlation::calculate_spearman_rank_correlation_coefficient(
                        fragment_data,
                        exp_data_up_[len_waveform-MIN_WAVEFORM_LEN],
                        coeff_up);

                    int err_dn = xtechnical_correlation::calculate_spearman_rank_correlation_coefficient(
                        fragment_data,
                        exp_data_dn_[len_waveform-MIN_WAVEFORM_LEN],
                        coeff_dn);
                    if(err_up != OK) return err_up;
                    if(err_dn != OK) return err_dn;
                    if(abs(coeff_up) > abs(coeff_dn)) {
                        out = coeff_up;
                    } else {
                        out = coeff_dn;
                    }
                    return OK;
                }
            }
            return err;
        }

        void clear() {
            iMW.clear();
        }
    };
//------------------------------------------------------------------------------
    /** \brief Класс для подсчета коррлеяции между валютными парами
     */
    template <typename T>
    class CurrencyCorrelation {
    private:
        std::vector<std::vector<T>> data_;
        std::vector<std::vector<T>> data_test_;
        size_t period_ = 0;
        bool is_test_ = false;
    public:
        enum CorrelationType {
            SPEARMAN_RANK = 0,
            PEARSON = 1,
        };
        /** \brief Инициализировать индикатор
         * \param period период индикатора
         * \param num_symbols колючество валютных пар
         */
        CurrencyCorrelation(const size_t &period, const size_t &num_symbols) {
            data_.resize(num_symbols);
            data_test_.resize(num_symbols);
            period_ = period;
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \param num_symbol номер валютной пары
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &in, const size_t &num_symbol) {
            is_test_ = false;
            if(period_ == 0) {
                return NO_INIT;
            }
            if(data_[num_symbol].size() < period_) {
                data_[num_symbol].push_back(in);
                if(data_[num_symbol].size() == period_) {
                    return OK;
                }
            } else {
                data_[num_symbol].push_back(in);
                data_[num_symbol].erase(data_[num_symbol].begin());
                return OK;
            }
            return INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \param num_symbol номер валютной пары
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T &in, const size_t &num_symbol) {
            is_test_ = true;
            if(period_ == 0) {
                return NO_INIT;
            }
            data_test_ = data_;
            if(data_test_[num_symbol].size() < period_) {
                data_test_[num_symbol].push_back(in);
                if(data_test_[num_symbol].size() == period_) {
                    return OK;
                }
            } else {
                data_test_[num_symbol].push_back(in);
                data_test_[num_symbol].erase(data_test_[num_symbol].begin());
                return OK;
            }
            return INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Посчитать корреляцию между двумя валютными парами
         * \param out значение корреляции
         * \param num_symbol_1 номер первой валютной пары
         * \param num_symbol_2 номер второй валютной пары
         * \param correlation_type тип корреляции (SPEARMAN_RANK, PEARSON)
         * \return состояние ошибки, 0 в случае успеха
         */
        int calculate_correlation(
                T &out,
                const size_t &num_symbol_1,
                const size_t &num_symbol_2,
                const size_t &correlation_type = SPEARMAN_RANK) {
            std::vector<T> norm_vec_1(period_), norm_vec_2(period_);
            if(is_test_) {
                if(data_test_[num_symbol_1].size() == (size_t)period_ &&
                    data_test_[num_symbol_2].size() == (size_t)period_) {
                    if(correlation_type == SPEARMAN_RANK) {
                        xtechnical_normalization::calculate_min_max(
                            data_test_[num_symbol_1],
                            norm_vec_1,
                            MINMAX_SIGNED);
                        xtechnical_normalization::calculate_min_max(
                            data_test_[num_symbol_2],
                            norm_vec_2,
                            MINMAX_SIGNED);
                        return xtechnical_correlation::calculate_spearman_rank_correlation_coefficient(
                            norm_vec_1,
                            norm_vec_2,
                            out);
                    } else
                    if(correlation_type == PEARSON) {
                        xtechnical_normalization::calculate_min_max(
                            data_test_[num_symbol_1],
                            norm_vec_1,
                            MINMAX_SIGNED);
                        xtechnical_normalization::calculate_min_max(
                            data_test_[num_symbol_2],
                            norm_vec_2,
                            MINMAX_SIGNED);
                        return xtechnical_correlation::calculate_pearson_correlation_coefficient(
                            norm_vec_1,
                            norm_vec_2,
                            out);
                    } else {
                        return INVALID_PARAMETER;
                    }
                }
            } else {
                if(data_[num_symbol_1].size() == (size_t)period_ &&
                    data_[num_symbol_2].size() == (size_t)period_) {
                    if(correlation_type == SPEARMAN_RANK) {
                        xtechnical_normalization::calculate_min_max(
                            data_[num_symbol_1],
                            norm_vec_1,
                            MINMAX_SIGNED);
                        xtechnical_normalization::calculate_min_max(
                            data_[num_symbol_2],
                            norm_vec_2,
                            MINMAX_SIGNED);
                        return xtechnical_correlation::calculate_spearman_rank_correlation_coefficient(
                            norm_vec_1,
                            norm_vec_2,
                            out);
                    } else
                    if(correlation_type == PEARSON) {
                        xtechnical_normalization::calculate_min_max(
                            data_[num_symbol_1],
                            norm_vec_1,
                            MINMAX_SIGNED);
                        xtechnical_normalization::calculate_min_max(
                            data_[num_symbol_2],
                            norm_vec_2,
                            MINMAX_SIGNED);
                        return xtechnical_correlation::calculate_pearson_correlation_coefficient(
                            norm_vec_1,
                            norm_vec_2,
                            out);
                    } else {
                        return INVALID_PARAMETER;
                    }
                }
            }
            return INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Найти коррелирующие валютные пары
         * \param symbol_1 список первой валютной пары в коррелирующей паре
         * \param symbol_2 список второй валютной пары в коррелирующей паре
         * \param threshold_coefficient порог срабатывания
         * для коэффициента корреляции
         * \param correlation_type тип корреляции (SPEARMAN_RANK, PEARSON)
         */
        void find_correlated_pairs(
                std::vector<size_t> &symbol_1,
                std::vector<size_t> &symbol_2,
                std::vector<T> &coefficient,
                T threshold_coefficient,
                const int correlation_type = SPEARMAN_RANK) {
            symbol_1.clear();
            symbol_2.clear();
            coefficient.clear();
            size_t data_test_size = data_test_.size();
            size_t data_test_size_dec = data_test_.size() - 1;
            if(is_test_) {
                for(size_t i = 0; i < data_test_size_dec; ++i) {
                    for(size_t j = i + 1; j < data_test_size; ++j) {
                        T coeff;
                        if(calculate_correlation(
                                coeff,
                                i,
                                j,
                                correlation_type) == OK) {
                            if(std::abs(coeff) > threshold_coefficient) {
                                symbol_1.push_back(i);
                                symbol_2.push_back(j);
                                coefficient.push_back(coeff);
                            } // if
                        } // if
                    } // for j
                } // for i
            } // if
        }

        /** \brief Очистить данные индикатора
         */
        void clear() {
            data_.clear();
            data_test_.clear();
        }
    };

    /** \brief Адаптивная скользящая средняя Кауфмана
     *
     * Технический индикатор, разновидность адаптивной скользящей средней,
     * построенной на базе экспоненциально сглаженной скользящей средней
     * и оригинальной методики определения
     * и применения волатильности в качестве динамически
     * изменяющейся сглаживающей константы
     */
    template <typename T>
    class AMA {
    private:
        uint32_t n;
        uint32_t f;
        uint32_t s;
        uint32_t period_std_dev;
        T coeff;
        T prev_ama = 0;
        T filter = 0;
        bool is_square;
        std::vector<T> data;
        MW<T> iMW;
        int err_std_dev = xtechnical_common::NO_INIT;
    public:

        AMA(const uint32_t period = 10,
            const uint32_t fast_ma_period = 2,
            const uint32_t slow_ma_period = 30,
            const uint32_t period_filter = 10,
            const T coeff_filter = 0.1,
            const bool is_square_smooth = true) :
            n(period),
            f(fast_ma_period),
            s(slow_ma_period),
            period_std_dev(period_filter),
            coeff(coeff_filter),
            is_square(is_square_smooth),
            iMW(period_filter){}

       int update(const T in, T &out) {
            data.push_back(in);
            if(data.size() > n) {
                T direction = std::abs(data[n - 1] - data[0]);
                T volume = 0;
                for(uint32_t i = n - 1; i > 0; i--) {
                    volume += std::abs(data[i] - data[i - 1]);
                }
                T er = direction/volume;
                T fastest = 2.0/(T)(f + 1);
                T slowest = 2.0/(T)(s + 1);
                T smooth = er * (fastest - slowest) + slowest;
                T c = is_square ? smooth * smooth : smooth;
                T temp = c * in + (1.0 - c) * prev_ama;
                T di = temp - prev_ama;
                prev_ama = temp;
                out = prev_ama;
                err_std_dev = iMW.update(di);
                if(err_std_dev != xtechnical_common::OK)
                    return xtechnical_common::OK;
                T std_dev_value = 0;
                iMW.get_std_dev(std_dev_value, period_std_dev);
                filter = coeff * std_dev_value;
                return xtechnical_common::OK;
            } else {
                prev_ama = in;
                filter = 0;
                out = in;
                return xtechnical_common::NO_INIT;
            }
        }

        int test(const T in, T &out) {
            std::vector<T> test_data = data;
            test_data.push_back(in);
            if(test_data.size() > n) {
                T direction = std::abs(test_data[n - 1] - test_data[0]);
                T volume = 0;
                for(uint32_t i = n - 1; i > 0; i--) {
                    volume += std::abs(test_data[i] - test_data[i - 1]);
                }
                T er = direction/volume;
                T fastest = 2.0/(T)(f + 1);
                T slowest = 2.0/(T)(s + 1);
                T smooth = er * (fastest - slowest) + slowest;
                T c = is_square ? smooth * smooth : smooth;
                T temp = c * in + (1.0 - c) * prev_ama;
                T di = temp - prev_ama;
                out = temp;
                err_std_dev = iMW.update(di);
                if(err_std_dev != xtechnical_common::OK)
                    return xtechnical_common::OK;
                T std_dev_value = 0;
                iMW.get_std_dev(std_dev_value, period_std_dev);
                filter = coeff * std_dev_value;
                return xtechnical_common::OK;
            } else {
                filter = 0;
                out = in;
                return xtechnical_common::NO_INIT;
            }
        }

        int get_filter(T &out) {
            out = filter;
            return err_std_dev;
        }

        /** \brief Очистить данные индикатора
         */
        void clear() {
            data.clear();
            iMW.clear();
            prev_ama = 0;
            filter = 0;
            err_std_dev = xtechnical_common::NO_INIT;
        }
    };

    /** \brief Скользящая средняя NoLagMa
     */
    template <typename T>
    class NoLagMa {
        private:
        T Pi = 3.14159265358979323846264338327950288;
        std::vector<T> nlm_values;
        std::vector<T> nlm_prices;
        std::vector<T> nlm_alphas;
        uint32_t _length = 0;
        uint32_t _len = 1;
        uint32_t _weight = 2;
        uint32_t LengthMA = 10;
        size_t bars = 0;
        int err = xtechnical_common::NO_INIT;

        inline T calc(const T price, const int length, int r) {
            /* прошу прощения за говнокод,
             * это копипаст из метатрейдера, переписанный на С++
             */
            if(nlm_prices.size() != bars) {
                nlm_prices.resize(bars);
            }
            nlm_prices[r] = price;
            if (nlm_values[_length] != length) {
                T Cycle = 4.0;
                T Coeff = 3.0*Pi;
                int    Phase = length-1;

                nlm_values[_length] = length;
                nlm_values[_len] = length*4 + Phase;
                nlm_values[_weight] = 0;

                if (nlm_alphas.size() < nlm_values[_len]) {
                    nlm_alphas.resize((size_t)nlm_values[_len]);
                }
                for (int k = 0; k< nlm_values[_len]; k++) {
                    T t;
                    if (k <= Phase-1) {
                        t = 1.0 * k/(Phase-1);
                    } else {
                        t = 1.0 + (k - Phase + 1)*(2.0 * Cycle - 1.0)/
                            (Cycle * (T)length - 1.0);
                    }
                    T beta = cos(Pi*t);
                    T g = 1.0/(Coeff*t+1);
                    if (t <= 0.5 ) {g = 1;}

                    nlm_alphas[k] = g * beta;
                    nlm_values[_weight] += nlm_alphas[k];
                }
            }

            if (nlm_values[_weight]>0) {
                double sum = 0;
                for (int k=0; k < nlm_values[_len] && (r-k)>=0; k++)
                    sum += nlm_alphas[k]*nlm_prices[r-k];
                err = xtechnical_common::OK;
                return(sum / nlm_values[_weight]);
            } else return 0;
        }

        public:

        NoLagMa(const uint32_t period = 10) {
            LengthMA = period;
            nlm_values.resize(3);
        }

        int update(const T in, T &out) {
            ++bars;
            out = calc(in, LengthMA, bars - 1);
            return err;
        }

        int test(const T in, T &out) {
            std::vector<T> old_nlm_values = nlm_values;
            std::vector<T> old_nlm_prices = nlm_prices;
            std::vector<T> old_nlm_alphas = nlm_alphas;
            out = calc(in, LengthMA, bars);
            nlm_values = old_nlm_values;
            nlm_prices = old_nlm_prices;
            nlm_alphas = old_nlm_alphas;
            bars = 0;
            return err;
        }

        void clear() {
            nlm_values.resize(3);
            nlm_prices.clear();
            nlm_alphas.clear();
            err = xtechnical_common::NO_INIT;
        }
    };

    /** \brief Гистограмма частот
     */
    template<class T>
    class FreqHist {
    private:
        MW<T> iMW;
        xtechnical_dft::DftReal<T> iDftReal;
        size_t dft_period = 0;
    public:

        FreqHist() {};

        FreqHist(const size_t period, const size_t window_type) :
            iMW(period), iDftReal(period, window_type) {
            dft_period = period;
        };

        int update(
                const T &input,
                std::vector<T> &histogram,
                const T sample_rate = 0) {
            int err = iMW.update(input);
            if(err != xtechnical_common::OK) return err;
            std::vector<T> buffer;
            iMW.get_data(buffer);
            std::vector<T> frequencies;
            xtechnical_normalization::calculate_min_max(
                buffer,
                buffer,
                xtechnical_common::MINMAX_SIGNED);
            return iDftReal.update(buffer, histogram, frequencies, sample_rate);
        }

        int update(
                const T &input,
                std::vector<T> &amplitude,
                std::vector<T> &frequencies,
                const T sample_rate = 0) {
            int err = iMW.update(input);
            if(err != xtechnical_common::OK) return err;
            std::vector<T> buffer;
            iMW.get_data(buffer);
            xtechnical_normalization::calculate_min_max(
                buffer,
                buffer,
                xtechnical_common::MINMAX_SIGNED);
            return iDftReal.update(buffer, amplitude, frequencies, sample_rate);
        }

        void clear() {
            iMW.clear();
        }
    };

    /** \brief Ro — темп изменения цены. Другое имя — Momentum
     *
     * Отличие в том, что в RoC линией баланса является «0», а в Momentum «100»
     * Формула:
     * Pt  — текущая цена
     * Pt-n — цена  n периодов назад
     * Momentum=Pn-Pn-1
     * RoCtn=((Momentum/Pt-n)*100)-100
     */
    template<class T>
    class RoC {
    private:
        std::vector<T> buffer;
        size_t buffer_size = 0;
    public:

        RoC() {};

        /** \brief Инициализировать скользящую сумму
         * \param period период
         */
        RoC(const size_t period) : buffer_size(period) {
            buffer.reserve(period);
        }

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in, T &out) {
            if(buffer_size == 0) {
                out = 0;
                return NO_INIT;
            }
            if(buffer.size() < buffer_size) {
                buffer.push_back(in);
                if(buffer.size() == buffer_size) {
                    if(buffer.front() == 0) out = buffer.back() > 0 ? 100 :
                        -100;
                    else out = ((buffer.back() - buffer.front()) /
                        buffer.front()) * 100.0;
                    return OK;
                }
            } else {
                buffer.push_back(in);
                buffer.erase(buffer.begin());
                if(buffer.front() == 0) out = buffer.back() > 0 ? 100 :
                        -100;
                else out = ((buffer.back() - buffer.front()) /
                    buffer.front()) * 100.0;
                return OK;
            }
            out = 0;
            return INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Протестировать индикатор
         *
         * Данный метод отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in, T &out) {
            if(buffer_size == 0) {
                out = 0;
                return NO_INIT;
            }
            std::vector<T> _buffer = buffer;
            if(_buffer.size() < buffer_size) {
                _buffer.push_back(in);
                if(_buffer.size() == buffer_size) {
                    if(_buffer.front() == 0) out = _buffer.back() > 0 ? 100 :
                        -100;
                    else out = ((_buffer.back() - _buffer.front()) /
                        _buffer.front()) * 100.0;
                    return OK;
                }
            } else {
                _buffer.push_back(in);
                _buffer.erase(_buffer.begin());
                if(_buffer.front() == 0) out = _buffer.back() > 0 ? 100 :
                    -100;
                else out = ((_buffer.back() - _buffer.front()) /
                    _buffer.front()) * 100.0;
                return OK;
            }
            out = 0;
            return INDICATOR_NOT_READY_TO_WORK;
        }

        /** \brief Очистить данные индикатора
         */
        void clear() {
            buffer.clear();
        }
    };

    /** \brief Индекс денежного потока
     */
    template <typename T, class INDICATOR_TYPE>
    class MFI {
    private:
        INDICATOR_TYPE iU;
        INDICATOR_TYPE iD;
        bool is_init_ = false;
        bool is_update_ = false;
        T prev_ = 0;
    public:
        MFI() {}

        /** \brief Инициализировать индикатор индекс денежного потока
         * \param period период индикатора
         */
        MFI(const size_t &period) : iU(period), iD(period) {
            is_init_ = true;
        }

        /** \brief Инициализировать индикатор индекса относительной силы
         * \param period период индикатора
         */
        void init(const size_t &period) {
            is_init_ = true;
            is_update_ = false;
            iU = INDICATOR_TYPE(period);
            iD = INDICATOR_TYPE(period);
        }

        /** \brief Обновить состояние индикатора
         * \param price Цена, в оригинале используется типичная цена
         * \param volume Объем торгов
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &price, const T &volume, T &out) {
            if(!is_init_) {
                return NO_INIT;
            }
            if(!is_update_) {
                prev_ = price;
                is_update_ = true;
                out = 50.0;
                return INDICATOR_NOT_READY_TO_WORK;
            }
            /* поток необработанных денег */
            T mf = price * volume;

            /* На основе денежного потока вычисляются положительный и отрицательный денежные потоки */
            T u = 0;
            T d = 0;
            if(prev_ < price) {
                u = mf;
            } else
            if(prev_ > price) {
                d = mf;
            }

            int erru, errd = 0;
            T mu = 0;
            T md = 0;
            erru = iU.update(u, mu);
            errd = iD.update(d, md);
            prev_ = price;
            if(erru != OK || errd != OK) {
                out = 50.0;
                return INDICATOR_NOT_READY_TO_WORK;
            }
            if(md == 0) {
                out = 100.0;
                return OK;
            }
            /* коэффициент денежного потока */
            T mfr = mu / md;
            out = 100.0 - (100.0 / (1.0 + mfr));
            return OK;
        }

        /** \brief Обновить состояние индикатора
         * \param high Наивысшая цена бара
         * \param low Наинизшая цена бара
         * \param close Цена закрытия бара
         * \param volume Объем торгов
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(
                const T &high,
                const T &low,
                const T &close,
                const T &volume,
                T &out) {
            const T price = (high + low + close) / 3.0;
            return update(price, volume, out);
        }

        /** \brief Обновить состояние индикатора
         * \param price Цена, в оригинале используется типичная цена
         * \param volume Объем торгов
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T &price, const T &volume) {
            if(!is_init_) {
                return NO_INIT;
            }
            if(!is_update_) {
                prev_ = price;
                is_update_ = true;
                return INDICATOR_NOT_READY_TO_WORK;
            }
            /* поток необработанных денег */
            T mf = price * volume;

            /* На основе денежного потока вычисляются положительный и отрицательный денежные потоки */
            T u = 0;
            T d = 0;
            if(prev_ < price) {
                u = mf;
            } else
            if(prev_ > price) {
                d = mf;
            }
            int erru, errd = 0;
            erru = iU.update(u, u);
            errd = iD.update(d, d);
            prev_ = price;
            if(erru != OK || errd != OK) {
                return INDICATOR_NOT_READY_TO_WORK;
            }
            return OK;
        }

        /** \brief Обновить состояние индикатора
         * \param high Наивысшая цена бара
         * \param low Наинизшая цена бара
         * \param close Цена закрытия бара
         * \param volume Объем торгов
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(
                const T &high,
                const T &low,
                const T &close,
                const T &volume) {
            const T price = (high + low + close) / 3.0;
            return update(price, volume);
        }

        /** \brief Протестировать индикатор
         *
         * Данная функция отличается от update тем,
         * что не влияет на внутреннее состояние индикатора
         * \param price Цена, в оригинале используется типичная цена
         * \param volume Объем торгов
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T &price, const T &volume, T &out) {
            if(!is_init_) {
                return NO_INIT;
            }
            if(!is_update_) {
                out = 50.0;
                return INDICATOR_NOT_READY_TO_WORK;
            }
            /* поток необработанных денег */
            T mf = price * volume;

            /* На основе денежного потока вычисляются положительный и отрицательный денежные потоки */
            T u = 0;
            T d = 0;
            if(prev_ < price) {
                u = mf;
            } else
            if(prev_ > price) {
                d = mf;
            }

            T mu = 0;
            T md = 0;
            int erru, errd = 0;
            erru = iU.test(u, mu);
            errd = iD.test(d, md);
            if(erru != OK || errd != OK) {
                out = 50.0;
                return INDICATOR_NOT_READY_TO_WORK;
            }
            if(d == 0) {
                out = 100.0;
                return OK;
            }
            /* коэффициент денежного потока */
            T mfr = mu / md;
            out = 100.0 - (100.0 / (1.0 + mfr));
            return OK;
        }

        /** \brief Протестировать индикатор
         * \param high Наивысшая цена бара
         * \param low Наинизшая цена бара
         * \param close Цена закрытия бара
         * \param volume Объем торгов
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(
                const T &high,
                const T &low,
                const T &close,
                const T &volume,
                T &out) {
            const T price = (high + low + close) / 3.0;
            return test(price, volume, out);
        }

        /** \brief Очистить данные индикатора
         */
        void clear() {
            is_update_ = false;
            iU.clear();
            iD.clear();
        }
    };

	/** \brief Индикатор осциллятор
     */
	template <class T, class INDICATOR_TYPE_1 = EMA<T>, class INDICATOR_TYPE_2 = EMA<T>, class INDICATOR_TYPE_3 = SMA<T>>
	class OsMa {
	private:
		INDICATOR_TYPE_1 iFastMA;
		INDICATOR_TYPE_2 iSlowMA;
		INDICATOR_TYPE_3 iSignalMA;
		DelayLine<T> iDelayLine;
	public:

		OsMa() {};

        /** \brief Конструктор индикатора осциллятора
         * \param period_fast Период быстрой скользящей средней
		 * \param period_slow Период медленной скользящей средней
		 * \param period_signal Период усреднения сигнальной линии
		 * \param shift Смещение назад
         */
        OsMa(	const size_t period_fast = 5,
				const size_t period_slow = 9,
				const size_t period_signal = 3,
				const size_t shift = 0) :
				iFastMA(period_fast),
				iSlowMA(period_slow),
				iSignalMA(period_signal),
				iDelayLine(shift) {}

        /** \brief Обновить состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(const T in, T &out) {
			out = 0;
			T fast_ma = in, slow_ma = in, signal_ma = in;
			int err_fast = iFastMA.update(in, fast_ma);
			int err_slow = iSlowMA.update(in, slow_ma);
			if(err_fast != OK || err_slow != OK) return NO_INIT;
			int err_signal = iSignalMA.update((fast_ma - slow_ma), signal_ma);
			if(err_signal != OK) return err_signal;
			return iDelayLine.update(signal_ma, out);
        }

		/** \brief Протестировать состояние индикатора
         * \param in сигнал на входе
         * \param out сигнал на выходе
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(const T in, T &out) {
			out = 0;
			T fast_ma = in, slow_ma = in, signal_ma = in;
			int err_fast = iFastMA.test(in, fast_ma);
			int err_slow = iSlowMA.test(in, slow_ma);
			if(err_fast != OK || err_slow != OK) return NO_INIT;
			int err_signal = iSignalMA.test((fast_ma - slow_ma), signal_ma);
			if(err_signal != OK) return err_signal;
			return iDelayLine.test(signal_ma, out);
        }

		void clear() {
			iFastMA.clear();
			iSlowMA.clear();
			iSignalMA.clear();
			iDelayLine.clear();
		}
	};

	template<class T>
	class MaBBandsYxf {
	private:
		OsMa<T> iOsMa;
		DelayLine<T> iDelayLineOsMa;
		SMA<T> iSmaHigh;
		SMA<T> iSmaLow;
		DelayLine<T> iDelayLineHigh;
		DelayLine<T> iDelayLineLow;
		WMA<T> iWmaHigh;
		WMA<T> iWmaLow;
		BollingerBands<T> iBbHigh;
		BollingerBands<T> iBbLow;
		double point = 0.00001;
		size_t dist_2 = 20;     /**<  */
    public:

        MaBBandsYxf(
                const size_t ma_period = 9,
                const size_t move_shift = 12,
                const size_t bb_period = 20,
                const double bb_factor = 0.4,
                const size_t os_period = 3,
                const double symbol_point = 0.00001,
                const size_t symbol_dist = 20) :
            iOsMa(5, 9, os_period), iDelayLineOsMa(1),
            iSmaHigh(ma_period), iSmaLow(ma_period),
            iDelayLineHigh(move_shift), iDelayLineLow(move_shift),
            iWmaHigh(4), iWmaLow(4),
            iBbHigh(bb_period, bb_factor), iBbLow(bb_period, bb_factor),
            point(symbol_point), dist_2(symbol_dist) {
        }

        /** \brief Обновить состояние индикатора
         * \param high Наивысшая цена бара
         * \param low Наинизшая цена бара
         * \param close Цена закрытия бара
         * \param out Прогноз, BUY == 1, SELL == 1
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int update(
                const T &high,
                const T &low,
                const T &close,
                T &out) {
			out = 0;
			T sma_high = high, sma_low = low, wma_high = high, wma_low = low;
			T bb_high_tl = high, bb_high_ml = high, bb_high_bl = high;
			T bb_low_tl = low, bb_low_ml = low, bb_low_bl = low;
			T os_ma_now = 0, os_ma_pre = 0;

			int err_sma_high = iSmaHigh.update(high, sma_high);
			int err_wma_high = iWmaHigh.update(high, wma_high);
			int err_bb_high = iBbHigh.update(high, bb_high_tl, bb_high_ml, bb_high_bl);

			int err_sma_low = iSmaLow.update(low, sma_low);
			int err_wma_low = iWmaLow.update(low, wma_low);
            int err_bb_low = iBbLow.update(low, bb_low_tl, bb_low_ml, bb_low_bl);

            int err_os_ma = iOsMa.update(close, os_ma_now);
            int err_os_ma_pre = NO_INIT;
            if(err_os_ma == OK) {
                err_os_ma_pre = iDelayLineOsMa.update(os_ma_now, os_ma_pre);
            }

            if(err_sma_high != OK || err_sma_low != OK ||
                err_os_ma != OK || err_os_ma_pre != OK) return NO_INIT;
            T maup1 = sma_high;
            T madn1 = sma_low;
            int err_maup1 = iDelayLineHigh.update(sma_high, maup1);
            int err_madn1 = iDelayLineLow.update(sma_low, madn1);
            if(err_maup1 != OK || err_madn1 != OK) return NO_INIT;

            T line_1 = 0, line_2 = 0;
            if(maup1 > bb_high_tl) {
                line_1 = maup1 + dist_2 * point;
                bb_high_tl = 0;
            } else if(maup1 < bb_high_tl) {
                line_1 = bb_high_tl;
                maup1 = 0;
            }

            if(madn1 > 0.0) {
                if(madn1 < bb_low_bl) {
                    line_2 = madn1 - dist_2 * point;
                    bb_low_bl = 0;
                } else if(madn1 > bb_low_bl) {
                    line_2 = bb_low_bl ;
                    madn1 = 0;
                }
            }
            if(madn1 == 0.0) {
                line_2 = bb_low_bl;
                madn1 = 0;
            }

            /* сигнал вверх */
            if((os_ma_now > 0 && os_ma_pre < 0) &&
                (wma_low < line_2) &&
                (low < line_2)) {
                out = BUY;
            }
            /* сигнал вниз */
            if((os_ma_now < 0 && os_ma_pre > 0) &&
                (wma_high > line_1) &&
                (high > line_1) ) {
                out = SELL;
            }
            return OK;
        }

        /** \brief Протестировать индикатор
         * \param high Наивысшая цена бара
         * \param low Наинизшая цена бара
         * \param close Цена закрытия бара
         * \param out Прогноз, BUY == 1, SELL == 1
         * \return вернет 0 в случае успеха, иначе см. ErrorType
         */
        int test(
                const T &high,
                const T &low,
                const T &close,
                T &out) {
			out = 0;
			T sma_high = high, sma_low = low, wma_high = high, wma_low = low;
			T bb_high_tl = high, bb_high_ml = high, bb_high_bl = high;
			T bb_low_tl = low, bb_low_ml = low, bb_low_bl = low;
			T os_ma_now = 0, os_ma_pre = 0;

			int err_sma_high = iSmaHigh.test(high, sma_high);
			int err_wma_high = iWmaHigh.test(high, wma_high);
			int err_bb_high = iBbHigh.test(high, bb_high_tl, bb_high_ml, bb_high_bl);

			int err_sma_low = iSmaLow.test(low, sma_low);
			int err_wma_low = iWmaLow.test(low, wma_low);
            int err_bb_low = iBbLow.test(low, bb_low_tl, bb_low_ml, bb_low_bl);

            int err_os_ma = iOsMa.test(close, os_ma_now);
            int err_os_ma_pre = NO_INIT;
            if(err_os_ma == OK) {
                err_os_ma_pre = iDelayLineOsMa.test(os_ma_now, os_ma_pre);
            }

            if(err_sma_high != OK || err_sma_low != OK ||
                err_os_ma != OK || err_os_ma_pre != OK) return NO_INIT;
            T maup1 = sma_high;
            T madn1 = sma_low;
            int err_maup1 = iDelayLineHigh.test(sma_high, maup1);
            int err_madn1 = iDelayLineLow.test(sma_low, madn1);
            if(err_maup1 != OK || err_madn1 != OK) return NO_INIT;

            T line_1 = 0, line_2 = 0;
            if(maup1 > bb_high_tl) {
                line_1 = maup1 + dist_2 * point;
                bb_high_tl = 0;
            } else if(maup1 < bb_high_tl) {
                line_1 = bb_high_tl;
                maup1 = 0;
            }

            if(madn1 > 0.0) {
                if(madn1 < bb_low_bl) {
                    line_2 = madn1 - dist_2 * point;
                    bb_low_bl = 0;
                } else if(madn1 > bb_low_bl) {
                    line_2 = bb_low_bl ;
                    madn1 = 0;
                }
            }
            if(madn1 == 0.0) {
                line_2 = bb_low_bl;
                madn1 = 0;
            }

            /* сигнал вверх */
            if((os_ma_now > 0 && os_ma_pre < 0) &&
                (wma_low < line_2) &&
                (low < line_2)) {
                out = BUY;
            }
            /* сигнал вниз */
            if((os_ma_now < 0 && os_ma_pre > 0) &&
                (wma_high > line_1) &&
                (high > line_1) ) {
                out = SELL;
            }
            return OK;
        }

        void clear() {
        	iSmaHigh.clear();
			iWmaHigh.clear();
			iBbHigh.clear();
			iSmaLow.clear();
			iWmaLow.clear();
            iBbLow.clear();
            iOsMa.clear();
            iDelayLineOsMa.clear();
            iDelayLineHigh.clear();
            iDelayLineLow.clear();
        }
	};

    /** \brief Мера склонности к чередовнию знаков (z-счет)
     *
     * Z - число СКО, на которое количество серий в выборке отклоняется
     * от своего математического ожидания
     * Если z > 3, то с вероятностью 0,9973 знаки имеют склонность к чередованию
     * Если z <-3, то с аналогичной вероятнсотью
     * проявляется склонность к сохранению знака
     * \param n общее число элементов в последовательности
     * \param r общее число серий положительных и отрицательных приращений
     * \param w общее число положительных приращений
     * \param l общее число отрицательных приращений
     * \return вернет Z
     */
    double calc_z_score(int n, int r, int w, int l) {
        double P = 2.0d * w * l;
        return (n * ((double)r - 0.5d) -  P) / sqrt((P * (P - (double)n))/
            ((double)n - 1.0d));
    }

    /** \brief Рассчитать долю капитала для стратегии
     * на основе меры склонности к чередовнию знаков
     * \param p вероятность правильного прогноза (от 0.0 до 1.0)
     * \param winperc процент выплаты брокера (от 0.0)
     * \return оптимальная доля капитала
     */
    double calc_z_scor_capital_share(double p, double winperc) {
        return p - (1.0 - p) * (1.0/winperc);
    }
}

#endif // INDICATORSEASY_HPP_INCLUDED
