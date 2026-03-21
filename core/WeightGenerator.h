#pragma once
#include <vector>
#include <cmath>
#include <iostream>
#include <iomanip>

class WeightGenerator
{
public:
    WeightGenerator() = default;
    explicit WeightGenerator(int cnt) : count_(cnt) {}

    template <typename T> requires std::is_arithmetic_v<T>
    void Apply(T& val, size_t& idx, double scaleFactor = 1.0)
    {
        if (idx >= weights_.size())
        {
            return;
        }

        val = static_cast<T>(static_cast<double>(val) * weights_[idx] * scaleFactor);
    }

    template <typename T> requires std::is_arithmetic_v<T>
    void Apply(std::vector<T>& vals, double scaleFactor = 1.0)
    {
        for (size_t i = 0; i < std::min(weights_.size(), vals.size()); i++)
        {
            vals[i] = static_cast<T>(static_cast<double>(vals[i]) * weights_[i] * scaleFactor);
        }
    }

    static WeightGenerator Gaussian(int count, double center = 0.5, double spread = 0.15)
    {
        return WeightGenerator{ count, GenerateGaussianWeights(count, center, spread) };
    }
    static WeightGenerator Linear(int count, bool flip = false)
    {
        return WeightGenerator{ count, GenerateLinearWeights(count, flip) };

    }
    static WeightGenerator Uniform(int count)
    {
        return WeightGenerator{ count, GenerateUniformWeights(count) };
    }

    void SetCount(int cnt) { count_ = cnt; }
    int GetCount() const { return count_; }

    WeightGenerator& ComputeGaussian(double center = 0.5, double spread = 0.15)
    {
        GenerateGaussianWeights(count_, center, spread).swap(weights_);
        return *this;
    }
    WeightGenerator& ComputeLinear(bool flip = false)
    {
        GenerateLinearWeights(count_, flip).swap(weights_);
        return *this;
    }
    WeightGenerator& ComputeUniform()
    {
        GenerateUniformWeights(count_).swap(weights_);
        return *this;
    }

    friend std::ostream& operator<<(std::ostream& os, const WeightGenerator& wg)
    {
        double maxWeight = *std::max_element(wg.weights_.begin(), wg.weights_.end());

        for (double weight : wg.weights_) 
        {
            int barLen = static_cast<int>(std::round((weight / maxWeight) * 25));
            os << std::setw(2) << barLen << " | " << std::string(barLen, '*') << "\n";
        }

        return os;
    }

    WeightGenerator(int cnt, std::vector<double>&& weights) : count_(cnt), weights_(std::move(weights)) {}

    static std::vector<double> GenerateGaussianWeights(int count, double center, double spread)
    {
        std::vector<double> weights(count);

        double sum = 0.0;

        for (int i = 0; i < count; ++i)
        {
            double normedX = static_cast<double>(i) / (count - 1);
            double weight = 0.0;

            if (std::abs(normedX - center) < spread * 3.0)
            {
                weight = std::exp(-0.5 * std::pow((normedX - center) / spread, 2));
            }

            weights[i] = weight;
            sum += weight;
        }

        if (sum != 0.0)
        {
            for (auto& w : weights)
            {
                w /= sum;
            }
        }

        return weights;
    }

    static std::vector<double> GenerateLinearWeights(int count, bool flip = false)
    {
        std::vector<double> weights(count);

        double step = 1.0 / static_cast<double>(count - 1);
        double sum = 0.0;

        for (int i = 0; i < count; ++i)
        {
            double val = step * i;
            weights[i] = flip ? (1.0 - val) : val;
            sum += weights[i];
        }

        if (sum != 0.0)
        {
            for (auto& w : weights)
            {
                w /= sum;
            }
        }

        return weights;
    }

    static std::vector<double> GenerateUniformWeights(int count)
    {
        std::vector<double> weights(count);

        double w = 1.0 / static_cast<double>(count);
        std::fill(weights.begin(), weights.end(), w);

        return weights;
    }

    static std::vector<double> GenerateDecayWeights(int count, double falloff, bool invert = false)
    {
        std::vector<double> weights(count);

        double sum = 0.0;

        for (int i = 0; i < count; ++i)
        {
            double x = static_cast<double>(i) / (count - 1); // 0 Å® 1

            if (invert)
            {
                x = 1.0 - x;  // mirror it
            }

            double weight = std::exp(-falloff * x);

            weights[i] = weight;
            sum += weight;
        }

        for (auto& w : weights)
        {
            w /= sum;
        }

        return weights;
    }

private:
    int count_ = 0;
    std::vector<double> weights_;
};