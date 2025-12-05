#include <stdio.h>
#include <string.h>

#define MAX_FOODS 20
#define MAX_ENTRIES 100
#define NAME_LEN 40


#define GOAL_GAIN 1
#define GOAL_LOSS 2
#define GOAL_MAINTAIN 3

#define ACT_SEDENTARY 1
#define ACT_LIGHT     2
#define ACT_MODERATE  3
#define ACT_ACTIVE    4
#define ACT_VERY      5

struct Food {
    char name[NAME_LEN];
    double kcal_per_100g;
    double protein_per_100g;
};

struct Entry {
    int food_index;
    double grams;
};


struct Food foods[MAX_FOODS] = {
    {"Boiled Rice",            130.0,  2.7},
    {"Chicken Breast (cooked)",165.0, 31.0},
    {"Egg (large, cooked)",    155.0, 13.0},
    {"Apple",                   52.0,  0.3},
    {"Banana",                  89.0,  1.1},
    {"Whole Milk (100ml)",      60.0,  3.2},
    {"Almonds",                579.0, 21.2},
    {"Oats (raw)",             389.0, 16.9},
    {"Paneer (cottage cheese)",265.0, 18.0},
    {"Wheat Bread (100g)",     265.0,  9.0},
    {"Salmon (cooked)",        208.0, 20.4},
    {"Lentils (cooked)",       116.0,  9.0},
    {"Potato (boiled)",         87.0,  2.0},
    {"Broccoli (cooked)",       35.0,  2.4},
    {"Peanut Butter",          588.0, 25.0}
};
int food_count = 15;

/* Entries recorded for the day */
struct Entry entries[MAX_ENTRIES];
int entry_count = 0;


void clear_input_buffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

/* --- Display food list --- */
void show_foods(void)
{
    int i;
    printf("\nAvailable foods (index : name — kcal/100g — protein/100g):\n");
    for (i = 0; i < food_count; i++) {
        printf("%2d: %-25s : %6.1f kcal : %5.1f g protein\n",
               i + 1, foods[i].name, foods[i].kcal_per_100g, foods[i].protein_per_100g);
    }
}

/* --- Add a food entry (user chooses index and grams) --- */
void add_food_entry(void)
{
    int idx;
    double grams;

    if (entry_count >= MAX_ENTRIES) {
        printf("Maximum entries reached for today.\n");
        return;
    }

    show_foods();
    printf("\nEnter food index (1-%d) to add or 0 to cancel: ", food_count);
    if (scanf("%d", &idx) != 1) {
        clear_input_buffer();
        printf("Invalid input.\n");
        return;
    }
    if (idx == 0) { clear_input_buffer(); return; }
    if (idx < 1 || idx > food_count) {
        clear_input_buffer();
        printf("Invalid index.\n");
        return;
    }

    printf("Enter amount consumed in grams (e.g. 150): ");
    if (scanf("%lf", &grams) != 1) {
        clear_input_buffer();
        printf("Invalid amount.\n");
        return;
    }
    if (grams <= 0.0) {
        clear_input_buffer();
        printf("Amount must be positive.\n");
        return;
    }

    entries[entry_count].food_index = idx - 1;
    entries[entry_count].grams = grams;
    entry_count++;

    clear_input_buffer();
    printf("Added: %.0f g of %s\n", grams, foods[idx - 1].name);
}

/* --- Calculate totals of calories and protein from entries --- */
void calculate_totals(double *total_kcal, double *total_protein)
{
    int i;
    double tk = 0.0, tp = 0.0;
    for (i = 0; i < entry_count; i++) {
        int fi = entries[i].food_index;
        double factor = entries[i].grams / 100.0;
        tk += foods[fi].kcal_per_100g * factor;
        tp += foods[fi].protein_per_100g * factor;
    }
    *total_kcal = tk;
    *total_protein = tp;
}

/* --- Estimate BMR --- */
double estimate_bmr(double weight_kg, double height_cm, int age, int sex_male)
{
    /* sex_male: 1 for male, 0 for female */
    if (sex_male) {
        return 10.0 * weight_kg + 6.25 * height_cm - 5.0 * age + 5.0;
    } else {
        return 10.0 * weight_kg + 6.25 * height_cm - 5.0 * age - 161.0;
    }
}

/* --- Activity multiplier (simple switch) --- */
double activity_multiplier(int activity_level)
{
    if (activity_level == ACT_SEDENTARY) return 1.2;
    if (activity_level == ACT_LIGHT)     return 1.375;
    if (activity_level == ACT_MODERATE)  return 1.55;
    if (activity_level == ACT_ACTIVE)    return 1.725;
    if (activity_level == ACT_VERY)      return 1.9;
    return 1.2;
}

/* --- Protein per kg recommendation based on goal --- */
double protein_per_kg_for_goal(int goal)
{
    if (goal == GOAL_GAIN)     return 1.8;
    if (goal == GOAL_LOSS)     return 1.8;
    /* maintenance */
    return 1.4;
}

/* --- Print today's food entries in a neat table --- */
void print_entries(void)
{
    int i;
    if (entry_count == 0) {
        printf("\nNo food entries recorded yet today.\n");
        return;
    }
    printf("\nToday's food entries:\n");
    printf("No.  Food                         Grams   kcal    Protein(g)\n");
    printf("---- ---------------------------  ------  ------  ----------\n");
    for (i = 0; i < entry_count; i++) {
        int fi = entries[i].food_index;
        double g = entries[i].grams;
        double kcal = foods[fi].kcal_per_100g * g / 100.0;
        double prot = foods[fi].protein_per_100g * g / 100.0;
        printf("%2d)  %-25s  %6.0f  %6.1f  %8.1f\n",
               i + 1, foods[fi].name, g, kcal, prot);
    }
}

/* --- Provide suggestions based on totals and user parameters --- */
void give_suggestions(double weight_kg, double height_cm, int age, int sex_male,
                      int activity_level, int goal,
                      double total_kcal, double total_protein)
{
    double bmr = estimate_bmr(weight_kg, height_cm, age, sex_male);
    double tdee = bmr * activity_multiplier(activity_level);
    double target_cal;

    if (goal == GOAL_GAIN) {
        target_cal = tdee + 300.0; 
    } else if (goal == GOAL_LOSS) {
        target_cal = tdee - 500.0; 
        if (target_cal < 1200.0) target_cal = 1200.0;
    } else {
        target_cal = tdee;
    }

    double prot_per_kg = protein_per_kg_for_goal(goal);
    double target_protein = prot_per_kg * weight_kg;

    printf("\n--- Personalized Summary & Suggestions ---\n");
    printf("Estimated BMR: %.0f kcal/day\n", bmr);
    printf("Estimated TDEE: %.0f kcal/day (with activity)\n", tdee);

    if (goal == GOAL_GAIN) printf("Goal: Muscle gain\n");
    else if (goal == GOAL_LOSS) printf("Goal: Fat loss\n");
    else printf("Goal: Maintenance\n");

    printf("Target calories: ~%.0f kcal/day\n", target_cal);
    printf("Protein target: ~%.0f g/day (%.1f g/kg)\n", target_protein, prot_per_kg);

    printf("\nToday consumed: %.0f kcal and %.1f g protein.\n", total_kcal, total_protein);

    /* Simple diet advice based on comparisons */
    if (total_kcal < target_cal - 200.0) {
        if (goal == GOAL_GAIN) {
            printf("- You are below kcal target for gaining. Increase portion sizes and add calorie-dense healthy foods (nuts, milk, oats, peanut butter).\n");
        } else if (goal == GOAL_LOSS) {
            printf("- Below target for fat loss; ensure energy is sufficient for daily activity.\n");
        } else {
            printf("- Below maintenance. Eat slightly more if unintentional.\n");
        }
    } else if (total_kcal > target_cal + 200.0) {
        if (goal == GOAL_LOSS) {
            printf("- You exceeded calorie target. Reduce energy-dense snacks and prefer vegetables, lean proteins.\n");
        } else if (goal == GOAL_GAIN) {
            printf("- Slight surplus is OK for gain; prefer protein-rich foods over empty calories.\n");
        } else {
            printf("- Above maintenance; adjust portions or activity to avoid weight gain.\n");
        }
    } else {
        printf("- You are close to your calorie target. Maintain consistency.\n");
    }

    /* Protein advice */
    if (total_protein + 0.1 < target_protein) {
        double need = target_protein - total_protein;
        printf("\nProtein advice: Need ~%.1f g more protein today.\n", need);
        printf("- Add 1-2 protein sources: chicken (100 g ~31 g), eggs, paneer, lentils, milk.\n");
    } else {
        printf("\nProtein advice: Protein target met. Good job for muscle maintenance/growth.\n");
    }

    /* Exercise advice (simple) */
    printf("\nExercise suggestions:\n");
    if (goal == GOAL_GAIN) {
        printf("- Focus on resistance training 3-5 times/week. Compound movements (squat, bench, row) and progressive overload.\n");
    } else if (goal == GOAL_LOSS) {
        printf("- Combine cardio (3-4 times/week) with resistance 2-3 times/week to preserve muscle.\n");
    } else {
        printf("- Balanced routine: 3 strength sessions + 2 cardio sessions weekly to maintain fitness.\n");
    }

    printf("\nPractical tips: Spread protein across meals, prefer whole foods, stay hydrated, sleep well, re-evaluate every 2-4 weeks.\n");
}


int main(void)
{
    double weight_kg = 0.0, height_cm = 0.0;
    int age = 20;
    int sex_choice = 1; /* 1 male, 2 female */
    int sex_male;
    int activity_level = ACT_MODERATE;
    int goal_choice;
    int choice;
    double total_kcal = 0.0, total_protein = 0.0;

    printf("=========================================\n");
    printf("   Health Tracking System\n");
    printf("=========================================\n\n");

    /* Collect basic user info */
    printf("Enter your weight in kg (e.g. 65): ");
    while (scanf("%lf", &weight_kg) != 1 || weight_kg <= 0.0) {
        clear_input_buffer();
        printf("Invalid. Enter weight in kg: ");
    }

    printf("Enter your height in cm (e.g. 170): ");
    while (scanf("%lf", &height_cm) != 1 || height_cm <= 0.0) {
        clear_input_buffer();
        printf("Invalid. Enter height in cm: ");
    }

    printf("Enter your age in years: ");
    while (scanf("%d", &age) != 1 || age <= 0) {
        clear_input_buffer();
        printf("Invalid. Enter age in years: ");
    }

    printf("Sex: 1) Male  2) Female : ");
    while (scanf("%d", &sex_choice) != 1 || (sex_choice != 1 && sex_choice != 2)) {
        clear_input_buffer();
        printf("Invalid. Sex: 1) Male  2) Female : ");
    }
    sex_male = (sex_choice == 1) ? 1 : 0;

    printf("\nActivity level:\n");
    printf(" 1) Sedentary (little or no exercise)\n");
    printf(" 2) Light (1-3 days/week)\n");
    printf(" 3) Moderate (3-5 days/week)\n");
    printf(" 4) Active (6-7 days/week)\n");
    printf(" 5) Very active (hard exercise or physical job)\n");
    printf("Choose (1-5): ");
    while (scanf("%d", &activity_level) != 1 || activity_level < 1 || activity_level > 5) {
        clear_input_buffer();
        printf("Invalid. Choose activity (1-5): ");
    }

    printf("\nFitness goal:\n 1) Muscle gain  2) Fat loss  3) Maintenance\nChoose (1-3): ");
    while (scanf("%d", &goal_choice) != 1 || goal_choice < 1 || goal_choice > 3) {
        clear_input_buffer();
        printf("Invalid. Choose goal (1-3): ");
    }

    clear_input_buffer();

    /* Menu loop */
    do {
        printf("\n\n========== Main Menu ==========\n");
        printf("1. Add food entry (what you ate today)\n");
        printf("2. Show today's entries\n");
        printf("3. Show totals & suggestions\n");
        printf("4. Reset today's entries\n");
        printf("0. Exit\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1) {
            clear_input_buffer();
            printf("Invalid choice.\n");
            continue;
        }
        clear_input_buffer();

        if (choice == 1) {
            add_food_entry();
        } else if (choice == 2) {
            print_entries();
        } else if (choice == 3) {
            calculate_totals(&total_kcal, &total_protein);
            printf("\nTotal today: %.1f kcal, %.1f g protein\n", total_kcal, total_protein);
            give_suggestions(weight_kg, height_cm, age, sex_male, activity_level,
                             (goal_choice == 1 ? GOAL_GAIN : (goal_choice == 2 ? GOAL_LOSS : GOAL_MAINTAIN)),
                             total_kcal, total_protein);
        } else if (choice == 4) {
            entry_count = 0;
            printf("Today's entries have been reset.\n");
        } else if (choice == 0) {
            printf("Exiting. Goodbye!\n");
        } else {
            printf("Invalid option. Choose again.\n");
        }
    } while (choice != 0);

    return 0;
}

