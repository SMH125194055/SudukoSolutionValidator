#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <omp.h>

using namespace std;
const int N = 9;
const int UNASSIGNED = -1;
class User
{
public:
  char name[50];
  int age;
  int pass;
  char score[10];
  double time1;
  double time2;

  User()
  {
    age = 0;
    pass = 0;
  }

  int setPass()
  {
    srand(time(NULL));
    int pas = 1001 + rand() % 9990;
    pass = pas;
    cout << "Your password  : " << pass;
    return pass;
  }

  char *setName()
  {
    char n[50];
    cout << "Enter your name : ";
    cin.sync();
    cin.getline(n, 49);
    strcpy(name, n);
    return name;
  }

  int setAge()
  {
    int a;
    cout << "Enter your age : ";
    cin >> a;
    age = a;
    return age;
  }

  char *Score(int c)
  {
    if (c == 1)
    {
      strcpy(score, "Win");
    }
    else if (c == 2)
    {
      strcpy(score, "Loss");
    }
    else if (c == 0)
    {
      strcpy(score, "NONE");
    }
    return score;
  }

  double get_time(double time, int t_num)
  {
    if (t_num == 1)
    {
      time1 = time;
      return time1;
    }
    else if (t_num == 2)
    {
      time2 = time;
      return time2;
    }
    return -1;
  }
  void setData();
  void SaveUser();
  void showData();
  void showUserData();
  void updateScore(int c);
  void updateTime(double, int);
};
void User::setData()
{
  setName();
  setAge();
  setPass();
}

void User::SaveUser()
{
  fstream fout;
  fout.open("UserData.dat", ios::out | ios::binary);
  setData();
  fout.write((char *)this, sizeof(*this));
  fout.close();
}

void User::showData()
{
  cout << endl;
  cout << "******************************************************";
  cout << endl
       << endl;
  cout << "Name : " << name << endl;
  cout << endl
       << "Age : " << age << endl;
  cout << endl
       << "Score : " << score << endl;
  cout << endl
       << "Time taken by single Thread : " << time1 << endl;
  cout << endl
       << "Time taken by Multiple Thread : " << time2 << endl;
  cout << endl
       << endl;
  cout << "******************************************************";
  cout << endl
       << endl;
}

void User::showUserData()
{
  fstream fin;
  int p;
  cout << "Enter your password : ";
  cin >> p;
  fin.open("UserData.dat", ios::in | ios::binary);
  if (!fin)
  {
    cout << "File not found!";
  }
  else
  {
    fin.read((char *)this, sizeof(*this));
    while (!fin.eof())
    {
      if (p == pass)
      {
        showData();
        break;
      }
      fin.read((char *)this, sizeof(*this));
    }
  }
  fin.close();
}

void User::updateScore(int c)
{
  fstream fin;
  fstream tempfin;
  tempfin.open("Temp.dat", ios::out | ios::binary);
  fin.open("UserData.dat", ios::in | ios::binary);
  while (!fin.eof())
  {
    fin.read((char *)this, sizeof(*this));
    Score(c);
    tempfin.write((char *)this, sizeof(*this));
  }
  fin.close();
  tempfin.close();
  remove("UserData.dat");
  rename("Temp.dat", "UserData.dat");
}

void User::updateTime(double time, int t_num)
{
  fstream fin;
  fstream tempfin;
  tempfin.open("Temp.dat", ios::out | ios::binary);
  fin.open("UserData.dat", ios::in | ios::binary);
  while (!fin.eof())
  {
    fin.read((char *)this, sizeof(*this));
    get_time(time, t_num);
    tempfin.write((char *)this, sizeof(*this));
  }
  fin.close();
  tempfin.close();
  remove("UserData.dat");
  rename("Temp.dat", "UserData.dat");
}

struct Sudoku
{
  int board[N][N];
};

int check_line(int input[N])
{
  int seen[10] = {0};
  int duplicateFound = 0;

#pragma omp parallel for reduction(+ : duplicateFound)
  for (int i = 0; i < N; i++)
  {
    int val = input[i];
    if (seen[val] != 0)
    {
      // Mark the value as seen
      seen[val] = 1;
        
      // Increment the duplicateFound flag
      duplicateFound++;
    }
  }

  return duplicateFound;
}

int ValidateGrid(int sudoku[N][N])
{
  int valid = 0;
#pragma omp parallel for schedule(dynamic)
  for (int i = 0; i < N; i++)
  {
    int temp_row = 3 * (i / 3);
    int temp_col = 3 * (i % 3);
    int seen[10] = {0};

    for (int k = temp_row; k < temp_row + 3; k++)
    {
      for (int l = temp_col; l < temp_col + 3; l++)
      {
        int val = sudoku[k][l];
        if (seen[val] != 0)
        {
          valid = 1;
          break; // Early exit if a subgrid is invalid
        }
        else
        {
          seen[val] = 1;
        }
      }
      if (valid)
      {
        break; // Early exit if a subgrid is invalid
      }
    }
  }

  return valid;
}

int ValidateSudokuSequential(int sudoku[N][N])
{
  User obj;
  double start_time, end_time, final_time;
  start_time = omp_get_wtime();
  for (int i = 0; i < N; i++)
  {
    if (check_line(sudoku[i]))
    {
      end_time = omp_get_wtime();
      final_time = end_time - start_time;
      obj.updateTime(final_time, 1);
      return 1;
    }

    int check_col[N];
    for (int j = 0; j < N; j++)
    {
      check_col[j] = sudoku[i][j];
    }
    if (check_line(check_col))
    {
      end_time = omp_get_wtime();
      obj.updateTime(final_time, 1);
      return 1;
    }
    if (ValidateGrid(sudoku))
    {
      end_time = omp_get_wtime();
      obj.updateTime(final_time, 1);
      return 1;
    }
  }

  return 0;
}

int ValidateSudokuParallel(int sudoku[N][N], int n_threads)
{
  int valid = 0;
  double start_time, end_time, final_time;
  start_time = omp_get_wtime();
  User obj;
#pragma omp parallel num_threads(n_threads)
#pragma omp parallel sections
  {
#pragma omp section
    {
      // Check rows
      for (int i = 0; i < N; i++)
      {
        if (check_line(sudoku[i]))
        {
          valid = 1;
          break; // Early exit if a row is invalid
        }
      }
    }

#pragma omp section
    {
      // Check columns
      for (int j = 0; j < N; j++)
      {
        int check_col[N];
        for (int i = 0; i < N; i++)
        {
          check_col[i] = sudoku[i][j];
        }
        if (check_line(check_col))
        {
          valid = 1;
          break; // Early exit if a column is invalid
        }
      }
    }

#pragma omp section
    {
      // Check subgrids
      if (!valid)
      {
        valid = ValidateGrid(sudoku);
      }
    }
  }
  end_time = omp_get_wtime();
  final_time = end_time - start_time;
  obj.updateTime(final_time, 2);

  return valid;
}

void printGrid(int grid[N][N])
{
  cout << endl;
  for (int row = 0; row < N; row++)
  {
    for (int col = 0; col < N; col++)
      cout << grid[row][col] << "\t";
    cout << endl
         << endl;
  }
}

//*****************************************************************//

void generateUnsolvedSudoku(int grid[N][N], int difficulty)
{
  srand(time(0));

  // Create an array of all positions to be removed
  int positions[N * N][2];
  for (int row = 0, k = 0; row < N; row++)
  {
    for (int col = 0; col < N; col++)
    {
      positions[k][0] = row;
      positions[k][1] = col;
      k++;
    }
  }

// Shuffle the array in parallel
#pragma omp parallel for
  for (int i = N * N - 1; i > 0; i--)
  {
    int j = rand() % (i + 1);

// Swap positions[i] and positions[j] using OpenMP's critical section
#pragma omp critical
    {
      swap(positions[i][0], positions[j][0]);
      swap(positions[i][1], positions[j][1]);
    }
  }

  // Remove numbers from the puzzle based on the difficulty level
  int cellsToRemove = 0;
  switch (difficulty)
  {
  case 1: // Easy
    cellsToRemove = 40;
    break;
  case 2: // Medium
    cellsToRemove = 50;
    break;
  case 3: // Hard
    cellsToRemove = 60;
    break;
  default:
    std::cout << "Invalid difficulty level." << std::endl;
    return;
  }

  // Initialize the grid with -1 (unassigned)
  for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
      grid[i][j] = UNASSIGNED;

// Assign numbers to some positions in parallel
#pragma omp parallel for schedule(dynamic)
  for (int i = 0; i < N * N - cellsToRemove; i++)
  {
    int row = positions[i][0];
    int col = positions[i][1];

    // Generate a random number (1 to 9) for assigned positions
    int num = rand() % 9 + 1;

// Atomically assign the number to the grid using critical section
#pragma omp critical
    {
      grid[row][col] = num;
    }
  }

  std::cout << "Generated Unsolved Sudoku Puzzle:" << std::endl;
  printGrid(grid);
}

///////*****************************************************************************************

//////*******************************************************************************************

int main()
{
  User obj;
  obj.SaveUser();

  int sudoku[9][9] =
      {
          {9, 4, 2, 1, 6, 3, 8, 5, 7},
          {5, 3, 6, 2, 8, 7, 9, 4, 1},
          {8, 7, 1, 9, 5, 4, 2, 3, 6},
          {3, 2, 7, 8, 1, 9, 4, 6, 5},
          {1, 5, 4, 3, 2, 6, 7, 9, 8},
          {6, 9, 8, 7, 4, 5, 1, 2, 3},
          {2, 6, 5, 4, 7, 1, 3, 8, 9},
          {7, 8, 9, 6, 3, 2, 5, 1, 4},
          {4, 1, 3, 5, 9, 8, 6, 7, 2}};
  printGrid(sudoku);
  /*
   int difficulty;
   cout << "\nEnter difficulty level (1: Easy, 2: Medium, 3: Hard): ";
   cin >> difficulty;
   generateUnsolvedSudoku(sudoku, difficulty);

   cout<<"If you want to solve by self press 1 else press 2\nenter choice: ";
   int choice;
   cin>>choice;
   if(choice==1)
   {
   for(int i=0; i<N; i++)
   {
     for(int j=0; j<N; j++)
     {
       if(sudoku[i][j]==-1)
       {
         cout<<"Enter your value for sudoku["<<i<<"]["<<j<<"] : ";
         cin>>sudoku[i][j];
         cout<<"\nUpdated sudoku : \n";
         printGrid(sudoku);
       }
     }
   }

 }
 else
 {
           cout << "Coming Soon";
 }
*/

  if (ValidateSudokuSequential(sudoku))
  {
    cout << "\nAnswer is validated by one thread: Invalid" << endl;
    obj.updateScore(2);
  }
  else
  {
    cout << "\nAnswer is validated by one thread: Valid" << endl;
    obj.updateScore(1);
  }

  cout << "How many threads you are wanting to validate sudoku's Solution : ";
  int n_threads;
  cin >> n_threads;

  if (ValidateSudokuParallel(sudoku, n_threads))
  {
    cout << "\nAnswer is validated by multithreading: Invalid" << endl;
    obj.updateScore(2);
  }
  else
  {
    cout << "\nAnswer is validated by multithreading: Valid" << endl;
    obj.updateScore(1);
  }

  int m;
  cout << "\nDo you want to see your stats? If yes, press 1; else, press 2: ";
  cin >> m;

  if (m == 1)
  {
    obj.showUserData();
  }

  return 0;
}
