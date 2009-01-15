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
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

namespace A5
{
    class A5Engine
    {
        public bool dbMode; //Debug mode

        private int nRegisters; //number of registers
        private int[] mRegLens; //max register lengths
        private int[] sArray; //source array
        private int[] rIndexes; //register indexes (clocking bits)
        private string[] pArray; //polynomials
        private int[][] registers; //registers

        private int[][] uExponents; //exponents being used

        /* --- Setting properties --- */
        public int numRegisters
        {
            get
            { return nRegisters; }
            set
            { nRegisters = value; }
        }

        public int[] maxRegLens
        {
            get
            { return mRegLens; }
            set
            { mRegLens = value; }
        }

        public int[] sourceArray
        {
            get
            { return sArray; }
            set
            { sArray = value; }
        }

        public int[] regIndexes
        {
            get
            { return rIndexes; }
            set
            { rIndexes = value; }
        }

        public string[] polynomialsArray
        {
            get
            { return pArray; }
            set
            { pArray = value; }
        }

        public int[][] Registers
        {
            get
            { return registers; }
            set
            { registers = value; }
        }

        /* --- Begin methods ---*/
        private void slowText(string message)
        {
            foreach (char c in message.ToCharArray())
            {
                Console.Write(c);
                System.Threading.Thread.Sleep(60);
            }

            Console.WriteLine();
        }

        public void Intro()
        {
            string message = "#################################################################\n";
            message += "#                      A5/1 Implementation                      #\n";
            message += "#                                                               #\n";
            message += "# Information: http://en.wikipedia.org/wiki/A5/1                #\n";
            message += "# Released:    24th October 2008                                #\n";
            message += "# App Options: -d [NumOfLoops] (Debugging mode)                 #\n";
            message += "#                                                               #\n";
            message += "# Written By: Brett Gervasoni (brett.gervasoni [at] gmail.com)  #\n";
            message += "#################################################################\n";

            Console.WriteLine(message);

            string matrix = "Now you will see how deep the rabit hole really goes...";
            slowText(matrix);

            System.Threading.Thread.Sleep(2500);
        }

        public int GetMaxRegLensTotal()
        {
            int total = 0;

            foreach (int len in mRegLens)
                total += len;

            return total;
        }

        private int XorValues(int val1, int val2)
        {
            int res = 0;

            if (val1 != val2)
                res = 1;

            return res;
        }

        private int XorRegValues(int[] vToXor)
        { 
            int final = 0;

            for (int i = 0; i < vToXor.Length; i++)
                final = XorValues(final, vToXor[i]);

            return final;
        }

        private int[][] RemoveIntArrElement(int[][] oArray, int index)
        {
            int[][] nArray = new int[oArray.Length-1][];

            for (int i = 0; i < oArray.Length; i++)
            {
                if (i != index)
                {
                    nArray[i-1] = new int[oArray[i].Length];
                    for (int x = 0; x < oArray[i].Length; x++)
                    {
                        nArray[i-1][x] = oArray[i][x];
                    }
                }
            }

            return nArray;
        }

        private int[][] ExtractPolyExponents(string[] polynomialsArray)
        {
            int[][] exponents = new int[polynomialsArray.Length][];

            for (int i = 0; i < polynomialsArray.Length; i++)
            {
                Regex expression = new Regex(@"x\^(\d+)");
                MatchCollection polyMatches = expression.Matches(polynomialsArray[i]);

                exponents[i] = new int[polyMatches.Count];

                for (int x = 0; x < polyMatches.Count; x++)
                {
                    //Console.WriteLine(polyMatches[x].Groups[1].ToString());
                    exponents[i][x] = int.Parse(polyMatches[x].Groups[1].ToString());
                }
            }

            return exponents;
        }

        private int FindLargestInt(int[] intArray)
        {
            int largest = 0;

            foreach (int num in intArray)
            {
                if (num > largest)
                    largest = num;
            }

            return largest;
        }

        private int[][] PolySelection()
        {
            int[][] exponents = ExtractPolyExponents(pArray);
            int[][] usingPolynomials = new int[nRegisters][];

            int counter = 0;
            int j = 0; //since i variable is reset
            for (int i = 0; i < exponents.Length; i++)
            {
                if (counter == nRegisters)
                    break;

                int largest = FindLargestInt(exponents[i]);

                if (largest < mRegLens[j])
                {
                    usingPolynomials[counter] = new int[exponents[i].Length];

                    for (int x = 0; x < exponents[i].Length; x++)
                        usingPolynomials[counter][x] = exponents[i][x];
                    
                    exponents = RemoveIntArrElement(exponents, i);

                    i = -1; //reset loop
                    counter++;
                }

                j++;
            }

            return usingPolynomials;
        }

        private int[] RegisterFill(int offset, int regNum)
        {
            int[] outArray = new int[regNum];

            for (int i = 0; i < regNum; i++)
            {
                outArray[i] = sArray[offset + i];
            }

            return outArray;
        }

        private int[] GetIndexValues()
        {
            int[] indexValues = new int[registers.Length];

            for (int i = 0; i < registers.Length; i++)
            {
                indexValues[i] = registers[i][rIndexes[i]];
            }

            return indexValues;
        }

        private int[] FindFrequency(int[] indexValues)
        {
            int[] tally = new int[2]; //size of 2 since its just binary

            foreach (int val in indexValues)
            {
                if (val == 0)
                    tally[0]++;
                else if (val == 1)
                    tally[1]++;
            }

            return tally;
        }

        public int[][] CreateRegisters()
        {
            int[][] filledRegisters = new int[nRegisters][];
            int offset = 0;

            //Does source array have enough data to fill?
            if (GetMaxRegLensTotal() <= sArray.Length)
            {
                for (int i = 0; i < nRegisters; i++)
                {
                    filledRegisters[i] = RegisterFill(offset, mRegLens[i]);
                    offset += mRegLens[i];
                }
            }

            uExponents = PolySelection();

            if (dbMode)
            {
                //Exponents in use
                int counter = 0;

                Console.WriteLine("[exponents]");
                foreach (int[] set in uExponents)
                {
                    Console.WriteLine("set: {0}", counter.ToString());

                    foreach (int exp in set)
                        Console.Write(exp.ToString() + " ");

                    Console.WriteLine();
                    counter++;
                }

                Console.WriteLine("[/exponents]");
            }

            return filledRegisters;
        }

        public int GetOutValue()
        {
            int[] vToXor = new int[registers.Length];
            int outValue = 0;

            for (int i = 0; i < registers.Length; i++)
                vToXor[i] = registers[i][0];

            outValue = XorRegValues(vToXor);

            return outValue;
        }

        public int[] RegistersToShift()
        {
            int[] indexValues = GetIndexValues();
            int[] tally = FindFrequency(indexValues);

            int highest = 0;
            int movVal = 0;

            if (dbMode)
            {
                Console.WriteLine("[indexValues]:");
                foreach (int v in indexValues)
                    Console.Write(v.ToString() + " ");
                Console.WriteLine("\n[/indexValues]:");

                Console.WriteLine("[tally]:");
                foreach (int v in tally)
                    Console.Write(v.ToString() + " ");
                Console.WriteLine("\n[/tally]:");
            }

            foreach (int count in tally)
            {
                if (count > highest)
                    highest = count;
            }

            for (int i = 0; i < tally.Length; i++)
            {
                if (tally[i] == highest)
                    movVal = i;
            }

            ArrayList regTS = new ArrayList();

            for (int i = 0; i < indexValues.Length; i++)
            {
                if (indexValues[i] == movVal)
                    regTS.Add(i);
            }

            return (int[])regTS.ToArray(typeof(int));
        }

        private int[] GetFeedbackValues(int[] regTS)
        {
            int[] regTSFBV = new int[regTS.Length]; //Reg To Shift Feed Back Values (regTSFBV)

            for (int i = 0; i < regTS.Length; i++)
            {
                int[] feedbackSet = new int[uExponents[regTS[i]].Length];

                for (int x = 0; x < uExponents[regTS[i]].Length; x++)
                {
                    feedbackSet[x] = registers[regTS[i]][uExponents[regTS[i]][x]];
                }

                regTSFBV[i] = XorRegValues(feedbackSet);
            }

            return regTSFBV;
        }

        public void RegisterShift(int[] regTS)
        {
            int[] shiftedElements = new int[regTS.Length];
            int[] regTSFBV = GetFeedbackValues(regTS);

            if (dbMode)
            {
                Console.WriteLine("[regTS]:");
                foreach (int v in regTS)
                    Console.Write(v.ToString() + " ");
                Console.WriteLine("\n[/regTS]:");

                Console.WriteLine("[regTSFBV]:");
                foreach (int v in regTSFBV)
                    Console.Write(v.ToString() + " ");
                Console.WriteLine("\n[/regTSFBV]:");
            }

            for (int i = 0; i < regTS.Length; i++)
            {
                int[] regShifting = registers[regTS[i]]; //Alias the register to shift

                shiftedElements[i] = registers[regTS[i]][0]; //Copy position zero value in registers to shift

                //Creates new register with appropriate max reg length
                int[] nRegister = new int[regShifting.Length]; //Could also use mRegLens[regTS[i]].Length

                //Fill values to length -1
                for (int x = 0; x < (regShifting.Length - 1); x++)
                    nRegister[x] = regShifting[x + 1]; //+1 Grabbing everything after position zero

                //Now put feedback values on the end (former RegisterPush method)
                nRegister[nRegister.Length - 1] = regTSFBV[i];

                registers[regTS[i]] = nRegister; //assign to register (update)
            }
        }
    }
}