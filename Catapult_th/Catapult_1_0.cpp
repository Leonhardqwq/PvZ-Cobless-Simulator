#include <array>
#include <cstdio>

std::array<double, 3000> catapult_prob(int ice){
    std::array<double, 3000> cnt = {};

    for (int frozen=399; frozen<=599; frozen++){
        for (int L=175;L<185;L++){
            int c = ice + frozen + 146;
            auto lo = (int)ceil(2.5 * (L/0.37f - ice + 1));
            auto hi = (int)ceil(2.5 * (L/0.23f - ice + 1));
            for (int k=lo;k<=hi;k++){
                auto v_lo = L / (k/2.5 + ice - 1);
                auto v_hi = L / ((k-1)/2.5 + ice - 1);
                v_lo = std::max(v_lo, 0.23);
                v_hi = std::min(v_hi, 0.37);
                cnt[k+c] += (v_hi - v_lo) / 0.14 / 2010;
            }
        }
    }

    return cnt;
}
std::array<double, 3000> catapult_prob_new(int ice){
    std::array<double, 3000> cnt = {};

    for (int frozen=399; frozen<=599; frozen++){
        for (int L=175;L<185;L++){
            // ice when moveing < v_i_1 <= ice when shooting < v_i_74 <= ice after shooting
            double v_i_1, v_i_74;
            if (ice<=1) v_i_1 = 0.37f;
            else        v_i_1 = 1.0*L/(ice-1);
            if (ice<=75) v_i_74 = 0.37f;
            else         v_i_74 = 1.0*L/(ice-74-1);

            if (v_i_1 < 0.23f) v_i_1 = 0.23f;
            if (v_i_1 > 0.37f) v_i_1 = 0.37f;
            if (v_i_74 < 0.23f) v_i_74 = 0.23f;
            if (v_i_74 > 0.37f) v_i_74 = 0.37f;

            // 0.23f <= v < v_i_1
            if(0.23f < v_i_1){
                int c = ice + frozen + 146;

                double v_lowerbound = 0.23f;
                double v_upperbound = v_i_1;

                auto lo = (int)ceil(2.5 * (L/v_upperbound - ice + 1));
                auto hi = (int)ceil(2.5 * (L/v_lowerbound - ice + 1));
                for (int k=lo;k<=hi;k++){
                    auto v_lo = L / (k/2.5 + ice - 1);
                    auto v_hi = L / ((k-1)/2.5 + ice - 1);
                    if(v_lo<v_lowerbound)  v_lo=v_lowerbound;
                    if(v_hi>v_upperbound) v_hi=v_upperbound;
                    cnt[k+c] += (v_hi - v_lo) / 0.14 / 2010;
                }
            }

            // v_i_1 <= v < v_i_74
            if(v_i_1<v_i_74){
                int c = -ice + frozen + 148;

                double v_lowerbound = v_i_1;
                double v_upperbound = v_i_74;

                auto lo = (int)ceil(1.0*L/v_upperbound);
                auto hi = (int)ceil(1.0*L/v_lowerbound);
                for (int k=lo;k<=hi;k++){
                    auto v_lo = 1.0*L / k;
                    auto v_hi = 1.0*L/(k-1);
                    if(v_lo<v_lowerbound)  v_lo=v_lowerbound;
                    if(v_hi>v_upperbound) v_hi=v_upperbound;
                    cnt[2*k+c] += (v_hi - v_lo) / 0.14 / 2010;
                }
            }

            // v_i_74 <= v <= 0.37f
            if(v_i_74<0.37f){
                int c = 74;

                double v_lowerbound = v_i_74;
                double v_upperbound = 0.37f;

                auto lo = (int)ceil(1.0*L/v_upperbound);
                auto hi = (int)ceil(1.0*L/v_lowerbound);
                for (int k=lo;k<=hi;k++){
                    auto v_lo = 1.0*L / k;
                    auto v_hi = 1.0*L/(k-1);
                    if(v_lo<v_lowerbound)  v_lo=v_lowerbound;
                    if(v_hi>v_upperbound) v_hi=v_upperbound;
                    cnt[k+c] += (v_hi - v_lo) / 0.14 / 2010;
                }
            }
        }
    }

    return cnt;
}

void query(){
    int ice;
    printf("ice time: ");
    scanf("%d", &ice);
    if(ice<1 || ice>473){
        printf("wrong ice time\n");
        return;
    }

    auto prob = catapult_prob(ice);
    for(int i=0;i<prob.size();i++){
        if(prob[i]!=0){
            printf("%d\n", i);
            break;
        }
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
    printf("%e\n\n", ans);    
}

void my_query(int ice,int t_lo, int t_hi){
    if(ice<1 || ice>473){
        printf("wrong ice time\n");
        return;
    }

    auto prob = catapult_prob(ice);
    for(int i=0;i<prob.size();i++){
        if(prob[i]!=0){
            printf("%d\n", i);
            break;
        }
    }

    if(t_lo>t_hi){
        printf("wrong range\n");
        return;
    }
    if(t_lo<0) t_lo=0;
    if(t_hi>=3000) t_hi=2999;

    double ans=0;
    for (int i=t_lo;i<=t_hi;i++)
        ans += prob[i];
    printf("%e\n\n", 1000000*ans);    
}

void my_query_new(int ice,int t_lo, int t_hi){
    if(ice<1){
        printf("wrong ice time\n");
        return;
    }

    auto prob = catapult_prob_new(ice);
    for(int i=0;i<prob.size();i++){
        if(prob[i]!=0){
            printf("%d\n", i);
            break;
        }
    }

    if(t_lo>t_hi){
        printf("wrong range\n");
        return;
    }
    if(t_lo<0) t_lo=0;
    if(t_hi>=3000) t_hi=2999;

    double ans=0;
    for (int i=t_lo;i<=t_hi;i++)
        ans += prob[i];
    printf("%e\n\n", 1000000*ans);    
}

int main(){
    // while (1)   query();
    int ice, t_hi;

    ice = 300; 
    t_hi = 1400;

    my_query(ice, 0, t_hi);
    my_query_new(ice, 0, t_hi);
    return 0;
}