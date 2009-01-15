/*
 * A5/1 Implementation
 * 
 * Information: http://en.wikipedia.org/wiki/A5/1
 * Released   : 24/10/2008
 * App Options: -d (Debugging mode)
 * 
 * Written by : Brett Gervasoni (brett.gervasoni [at] gmail.com)
 * Thanks to  : Angus McMahon (angusmcmahon [at] gmail.com)
 * 
 * Notes: 
 * - Debugging mode will show step by step processes
 * - The code has been designed to be easily expandable, so that more secure
 *   versions of the A5 family can be introduced. 
 * - Various properties of the A5Engine class can be modified
 * - If order matters then put things in order to begin with!
 *   Polynomials, clocking bits, and maximum register lengths.
 * - If data is not in order the application will try its best to make
 *   the cipher work. 
 * - Polynomials for each register are chosen based on their largest exponent
 *   being less than register length. 
 *   
 * Instructions:
 * numRegisters     = The number of registers to use
 * maxRegLens       = The max register length for each register
 * regIndexes       = The clocking bits for each register
 * polynomialsArray = The polynomials to use for each register
 * sourceArray      = Random binary bits
 * */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace A5
{
    class Program
    {
        static void Main(string[] args)
        {
            //Create registers
            A5Engine A5eng = new A5Engine();

            //To expand the algorithm...
            //Edit below properties
            A5eng.numRegisters = 3; //Number of registers to use
            A5eng.maxRegLens = new int[] { 19, 22, 23 }; //Test max reg lengths: { 5,5,5 };
            A5eng.regIndexes = new int[] { 8, 10, 10 }; //Test clocking bits: { 0,0,0 };

            //Test polynomials: { "x^1+x^2+x^3+1", "x^0+x^2+x^3+1", "x^3+x^4+1" };
            A5eng.polynomialsArray = new string[] { "x^8+x^17+x^16+x^13+1",
                                                "x^21+x^20+1",
                                                "x^22+x^21+x^20+x^7+1" };
            A5eng.sourceArray = new int[] { 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1,
                                            1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1,
                                            1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1,
                                            1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1,
                                            1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1,
                                            0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1 };

            /* DO NOT EDIT BELOW THIS POINT */
            int numRegPushes = 100;
            bool testPassed = true; //For cipher check

            if (args.Length > 0)
            {
                if (args[0] == "-d")
                {
                    A5eng.dbMode = true;
                    if (args.Length > 1)
                    {
                        try
                        {
                            numRegPushes = int.Parse(args[1]);
                        }
                        catch (Exception ex)
                        {
                            testPassed = false;

                            Console.WriteLine("Error: Numeric values only!");
                            Console.WriteLine("Exception: " + ex.Message);
                        }
                    }
                }
            }

            if (A5eng.sourceArray.Length < A5eng.GetMaxRegLensTotal())
            {
                testPassed = false;
                Console.WriteLine("[-] Not enough source data!");
            }

            if (A5eng.polynomialsArray.Length != A5eng.numRegisters)
            {
                testPassed = false;
                Console.WriteLine("[-] Not enough polynomials");
            }

            if (testPassed)
            {
                A5eng.Registers = A5eng.CreateRegisters();

                if (A5eng.dbMode)
                {
                    Console.WriteLine("Output (debugging mode): ");
                    for (int ia = 0; ia < numRegPushes; ia++)
                    {
                        Console.WriteLine("[register]");
                        int c = 0;
                        foreach (int[] p in A5eng.Registers)
                        {
                            Console.Write("register: {0} ", c);
                            foreach (int poly in p)
                            {
                                Console.Write(poly.ToString());
                            }
                            Console.WriteLine();

                            c++;
                        }
                        Console.WriteLine("[/register]");

                        int[] regTS = A5eng.RegistersToShift();
                        A5eng.RegisterShift(regTS);

                        System.Threading.Thread.Sleep(20); //Slow the output
                    }

                    Console.WriteLine("\n{0} loops of A5/1 have been completed.", numRegPushes.ToString());
                }
                else
                {
                    A5eng.Intro();

                    Console.WriteLine("Output: ");
                    while (true)
                    {
                        Console.Write(A5eng.GetOutValue().ToString());

                        int[] regTS = A5eng.RegistersToShift();
                        A5eng.RegisterShift(regTS);

                        System.Threading.Thread.Sleep(20); //Slow the output
                    }
                }
            }
        }
    }
}
