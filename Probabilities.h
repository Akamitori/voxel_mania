#ifndef PROBABILITIES_H
#define PROBABILITIES_H

#include <vector>
#include <utility>
#include "RandomNumberGeneration.h"


bool chance(RandomNumberGeneration& rng, float chance);
bool chance(RandomNumberGeneration& rng, double chance);


template <class T> T pick_a_weighted_item(RandomNumberGeneration& rng, const std::vector<std::pair<T,float>>& items){
    float weight_total=0;
    
    std::vector<std::pair<T,float>> items_with_probabilities{};
    for( const auto& [item,weight] : items){
		weight_total+=weight;
    }
    
    for( const auto& [item,weight] : items){
		items_with_probabilities.push_back({item, weight/weight_total});
    }
    
    
    for(const auto& [item,probability] : items_with_probabilities){
       if(chance(rng,probability)){
          return item;       
       }
    }
    
    return items_with_probabilities.back().first;
}


#endif //PROBABILITIES_H
