#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_RED "\033[31m"
#define COLOR_BOLD "\033[1m"

#define MAX_HASHES 100
#define MAX_COLLISIONS 10

typedef struct {
	char input[256];
	uint32_t hash;
	char collision_with[256];
	size_t attempts;
} HashResult;

HashResult hash_results[MAX_HASHES];
int hash_count = 0;
int collision_count = 0;

size_t total_strings_hashed = 0;
size_t total_collisions = 0;

uint32_t advanced_hash_verbose(const char *str, int verbose) {
	uint32_t hash = 0x811c9dc5;
	uint32_t prime = 16777619;

	if (verbose) {
		printf(COLOR_GREEN "Initial Hash: 0x%x (Base-10: %u, Binary: 0b", hash, hash);
		for (int i = 31; i >= 0; i--) {
			printf("%d", (hash >> i) & 1);
		}
		printf(")\n" COLOR_RESET);
	}

	while (*str) {
		if (verbose) {
			printf(COLOR_YELLOW "\nProcessing character: '%c' (ASCII: %d)\n" COLOR_RESET, *str, *str);
		}

		hash ^= (uint32_t)(*str);
		if (verbose) {
			printf(COLOR_BLUE "After XOR: 0x%x (Base-10: %u, Binary: 0b", hash, hash);
			for (int i = 31; i >= 0; i--) {
				printf("%d", (hash >> i) & 1);
			}
			printf(")\n" COLOR_RESET);
		}

		hash *= prime;
		if (verbose) {
			printf(COLOR_GREEN "After multiply by prime: 0x%x (Base-10: %u, Binary: 0b", hash, hash);
			for (int i = 31; i >= 0; i--) {
				printf("%d", (hash >> i) & 1);
			}
			printf(")\n" COLOR_RESET);
		}

		hash ^= (hash >> 13);
		if (verbose) {
			printf(COLOR_YELLOW "After XOR with right shift: 0x%x (Base-10: %u, Binary: 0b", hash, hash);
			for (int i = 31; i >= 0; i--) {
				printf("%d", (hash >> i) & 1);
			}
			printf(")\n" COLOR_RESET);
		}

		hash *= 0x5bd1e995;
		if (verbose) {
			printf(COLOR_BLUE "After multiply by 0x5bd1e995: 0x%x (Base-10: %u, Binary: 0b", hash, hash);
			for (int i = 31; i >= 0; i--) {
				printf("%d", (hash >> i) & 1);
			}
			printf(")\n" COLOR_RESET);
		}

		hash ^= (hash >> 15);
		if (verbose) {
			printf(COLOR_GREEN "After final XOR with right shift: 0x%x (Base-10: %u, Binary: 0b", hash, hash);
			for (int i = 31; i >= 0; i--) {
				printf("%d", (hash >> i) & 1);
			}
			printf(")\n" COLOR_RESET);
		}

		str++;
	}

	if (verbose) {
		printf(COLOR_RED "\nFinal Hash: 0x%x (Base-10: %u, Binary: 0b", hash, hash);
		for (int i = 31; i >= 0; i--) {
			printf("%d", (hash >> i) & 1);
		}
		printf(")\n" COLOR_RESET);
	}

	return hash;
}

void print_progress_bar(size_t current, size_t total) {
	int progress = (current * 100) / total;
	printf("\r[");
	for (int i = 0; i < 20; i++) {
		if (i < progress / 5) {
			printf("#");
		} else {
			printf("-");
		}
	}
	printf("] %d%% (%zu/%zu)", progress, current, total);
	fflush(stdout);
}

void find_collision() {
	const size_t max_attempts = 1000000;
	const size_t hash_table_size = 1000003;
	uint32_t *hash_table = calloc(hash_table_size, sizeof(uint32_t));
	char (*strings)[17] = calloc(hash_table_size, sizeof(char[17]));

	if (!hash_table || !strings) {
		printf(COLOR_RED "Memory allocation failed.\n" COLOR_RESET);
		free(hash_table);
		free(strings);
		return;
	}

	char str[17];
	size_t attempt = 0;

	printf(COLOR_YELLOW "\nFinding a collision...\n" COLOR_RESET);

	while (attempt < max_attempts) {
		size_t length = (rand() % 12) + 1;
		for (size_t i = 0; i < length; i++) {
			str[i] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()"[rand() % 72];
		}
		str[length] = '\0';

		printf(COLOR_BLUE "\nAttempt %zu: Current String: %s\n" COLOR_RESET, attempt + 1, str);
		uint32_t hash = advanced_hash_verbose(str, 1);

		uint32_t index = hash % hash_table_size;
		if (hash_table[index] != 0 && strcmp(strings[index], str) != 0) {
			printf(COLOR_GREEN "\nCollision found!\n" COLOR_RESET);
			printf(COLOR_BOLD "String 1: %s | String 2: %s | Hash: %u\n" COLOR_RESET, str, strings[index], hash);

			strcpy(hash_results[collision_count].input, str);
			hash_results[collision_count].hash = hash;
			strcpy(hash_results[collision_count].collision_with, strings[index]);
			hash_results[collision_count].attempts = attempt;
			collision_count++;

			total_collisions++;

			free(hash_table);
			free(strings);
			return;
		}

		hash_table[index] = hash;
		strcpy(strings[index], str);

		attempt++;

		if (attempt % 100 == 0) {
			print_progress_bar(attempt, max_attempts);
		}

		usleep(100);
	}

	printf(COLOR_RED "\nNo collision found within %zu attempts.\n" COLOR_RESET, max_attempts);

	free(hash_table);
	free(strings);
}

void reverse_lookup(uint32_t target_hash) {
	const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	const size_t charset_size = sizeof(charset) - 1;
	const size_t max_length = 5;
	char str[max_length + 1];
	str[max_length] = '\0';

	printf(COLOR_YELLOW "\nAttempting reverse lookup for hash: %u\n" COLOR_RESET, target_hash);

	for (size_t length = 1; length <= max_length; length++) {
		size_t indices[length];
		memset(indices, 0, sizeof(indices));

		while (1) {
			for (size_t i = 0; i < length; i++) {
				str[i] = charset[indices[i]];
			}
			str[length] = '\0';

			printf(COLOR_BLUE "\nAttempting string: %s\n" COLOR_RESET, str);
			uint32_t hash = advanced_hash_verbose(str, 1);

			if (hash == target_hash) {
				printf(COLOR_GREEN "\nMatch found! String: %s\n" COLOR_RESET, str);
				return;
			}

			size_t i = 0;
			while (i < length && ++indices[i] == charset_size) {
				indices[i] = 0;
				i++;
			}

			if (i == length) {
				break;
			}
		}
	}

	printf(COLOR_RED "\nNo string found for the given hash within the maximum length of %zu characters.\n" COLOR_RESET, max_length);
}

void save_results() {
	FILE *file = fopen("hash_results.txt", "w");
	if (!file) {
		printf(COLOR_RED "Error saving results to file.\n" COLOR_RESET);
		return;
	}

	fprintf(file, "Hash Results:\n");
	for (int i = 0; i < collision_count; i++) {
		fprintf(file, "%d. Input: %s | Hash: %u\n", i + 1, hash_results[i].input, hash_results[i].hash);
		if (strlen(hash_results[i].collision_with) > 0) {
			fprintf(file, "   Collided with: %s\n", hash_results[i].collision_with);
		}
	}
	fclose(file);
	printf(COLOR_GREEN "Results saved to 'hash_results.txt'.\n" COLOR_RESET);
}

void display_menu() {
	printf("\n1. Hash a string");
	printf("\n2. Compare two strings");
	printf("\n3. Find a collision");
	printf("\n4. Reverse hash lookup");
	printf("\n5. View summary");
	printf("\n6. View statistics");
	printf("\n7. Save results");
	printf("\n8. Exit\n");
	printf("Choose an option: ");
}

int main() {
	char input[256];
	char input2[256];

	srand(time(NULL));
	printf(COLOR_BOLD "Hashing Tests\n" COLOR_RESET);

	while (1) {
		display_menu();

		int choice;
		scanf("%d", &choice);
		getchar();

		switch (choice) {
			case 1:
				printf("\nEnter a string to hash: ");
				fgets(input, sizeof(input), stdin);
				input[strcspn(input, "\n")] = '\0';

				printf(COLOR_BOLD "\nVerbose Hashing Display:\n" COLOR_RESET);
				advanced_hash_verbose(input, 1);
				total_strings_hashed++;
				break;

			case 2:
				printf("\nEnter the first string: ");
				fgets(input, sizeof(input), stdin);
				input[strcspn(input, "\n")] = '\0';

				printf("Enter the second string: ");
				fgets(input2, sizeof(input2), stdin);
				input2[strcspn(input2, "\n")] = '\0';

				printf(COLOR_BOLD "\nComparing hashes...\n" COLOR_RESET);
				uint32_t hash1 = advanced_hash_verbose(input, 0);
				uint32_t hash2 = advanced_hash_verbose(input2, 0);

				if (hash1 == hash2) {
					printf(COLOR_GREEN "Hashes match! Possible collision.\n" COLOR_RESET);
				} else {
					printf(COLOR_RED "Hashes do not match.\n" COLOR_RESET);
				}
				break;

			case 3:
				find_collision();
				break;

			case 4:
				printf("\nEnter the hash to reverse lookup (Base-10): ");
				uint32_t target_hash;
				scanf("%u", &target_hash);
				getchar();
				reverse_lookup(target_hash);
				break;

			case 5:
				printf(COLOR_BOLD "\nSummary of Hashes:\n" COLOR_RESET);
				for (int i = 0; i < collision_count; i++) {
					printf("%d. Input: %s | Hash: %u\n", i + 1, hash_results[i].input, hash_results[i].hash);
					if (strlen(hash_results[i].collision_with) > 0) {
						printf(COLOR_RED "   Collided with: %s\n" COLOR_RESET, hash_results[i].collision_with);
					}
				}
				break;

			case 6:
				printf(COLOR_BOLD "\nProgram Statistics:\n" COLOR_RESET);
				printf("Total Strings Hashed: %zu\n", total_strings_hashed);
				printf("Total Collisions Found: %zu\n", total_collisions);
				if (collision_count > 0) {
					printf("Fastest Collision: %zu attempts\n", hash_results[0].attempts);
				} else {
					printf("No collisions found yet.\n");
				}
				break;

			case 7:
				save_results();
				break;

			case 8:
				printf(COLOR_GREEN "Exiting program. Goodbye!\n" COLOR_RESET);
				return 0;

			default:
				printf(COLOR_RED "Invalid choice! Try again.\n" COLOR_RESET);
		}
	}
}
