#include <cstdio>
#include <iostream>
#include "../inc/util.hpp"

std::vector<double> catapult_prob(int ice){
    std::vector<double> cnt(3000, 0.0);
    for (int frozen=399; frozen<=599; frozen++){
        for (int L=175;L<185;L++){
            // ice when moveing < v1 <= ice when shooting < v2 <= ice after shooting
            double v1, v2;
            if (ice<=1) v1 = 0.37f;
            else        v1 = 1.0*L/(ice-1);
            if (ice<=75) v2 = 0.37f;
            else         v2 = 1.0*L/(ice-74-1);
            if (v1 < 0.23f) v1 = 0.23f;
            if (v1 > 0.37f) v1 = 0.37f;
            if (v2 < 0.23f) v2 = 0.23f;
            if (v2 > 0.37f) v2 = 0.37f;
            // 0.23f <= v < v1
            if(0.23f < v1){
                // printf("when moving\n");
                int c = ice + frozen + 146;
                double v_lowerbound = 0.23f;
                double v_upperbound = v1;
                auto lo = (int)ceil(2.5 * (L/v_upperbound - ice + 1));
                auto hi = (int)ceil(2.5 * (L/v_lowerbound - ice + 1));
                for (int k=lo;k<=hi;k++){
                    auto v_lo = L / (k/2.5 + ice - 1);
                    auto v_hi = L / ((k-1)/2.5 + ice - 1);
                    if(v_lo<v_lowerbound)  v_lo=v_lowerbound;
                    if(v_hi>v_upperbound) v_hi=v_upperbound;
                    cnt[k+c] += (v_hi - v_lo) / (0.37f-0.23f) / 2010;
                }
            }
            // v1 <= v < v2
            if(v1<v2){
                // printf("when shooting\n");
                int c = -ice + frozen + 148;
                double v_lowerbound = v1;
                double v_upperbound = v2;
                auto lo = (int)ceil(1.0*L/v_upperbound);
                auto hi = (int)ceil(1.0*L/v_lowerbound);
                for (int k=lo;k<=hi;k++){
                    auto v_lo = 1.0*L / k;
                    auto v_hi = 1.0*L/(k-1);
                    if(v_lo<v_lowerbound)  v_lo=v_lowerbound;
                    if(v_hi>v_upperbound) v_hi=v_upperbound;
                    cnt[2*k+c] += (v_hi - v_lo) / (0.37f-0.23f) / 2010;
                }
            }
            // v2 <= v <= 0.37f
            if(v2<0.37f){
                // printf("after shooting\n");
                int c = 74;
                double v_lowerbound = v2;
                double v_upperbound = 0.37f;
                auto lo = (int)ceil(1.0*L/v_upperbound);
                auto hi = (int)ceil(1.0*L/v_lowerbound);
                for (int k=lo;k<=hi;k++){
                    auto v_lo = 1.0*L / k;
                    auto v_hi = 1.0*L/(k-1);
                    if(v_lo<v_lowerbound)  v_lo=v_lowerbound;
                    if(v_hi>v_upperbound) v_hi=v_upperbound;
                    cnt[k+c] += (v_hi - v_lo) / (0.37f-0.23f) / 2010;
                }
            }
        }
    }
    write_vector_to_csv(cnt, "result.csv");
    return cnt;
}

void query(){
    int ice;
    printf("ice time: ");
    scanf("%d", &ice);
    if(ice<1){
        printf("wrong ice time\n");
        return;
    }

    auto prob = catapult_prob(ice);
    for(int i=0;i<prob.size();i++)
        if(prob[i]!=0){
            printf("Earliest shooting time: %d\n", i);
            break;
        }
    
    int t_lo, t_hi;
    printf("[t_lo, t_hi]: ");
    scanf("%d %d", &t_lo, &t_hi);
    if(t_lo>t_hi){
        printf("wrong range\n");
        return;
    }
    if(t_lo<0) t_lo=0;
    if(t_hi>=3000) t_hi=2999;

    double ans=0;
    for (int i=t_lo;i<=t_hi;i++)
        ans += prob[i];
    printf("Probability: %e\n\n", ans);
}

void myquery(int ice, int t_hi){
    printf("ice time: %d\n", ice);
    if(ice<1){
        printf("wrong ice time\n");
        return;
    }

    auto prob = catapult_prob(ice);
    for(int i=0;i<prob.size();i++)
        if(prob[i]!=0){
            printf("Earliest shooting time: %d\n", i);
            break;
        }
    
    int t_lo=0;
    printf("[t_lo, t_hi]: %d %d\n", t_lo, t_hi);
    if(t_lo>t_hi){
        printf("wrong range\n");
        return;
    }
    if(t_lo<0) t_lo=0;
    if(t_hi>=3000) t_hi=2999;

    double ans=0;
    for (int i=t_lo;i<=t_hi;i++)
        ans += prob[i];
    printf("Probability: %e\n\n", ans);
}

int main(){
    while (1)   query();
    // myquery(3000, 548);
    return 0;
}