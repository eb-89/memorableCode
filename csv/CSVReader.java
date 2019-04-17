/* 
*  A class for holding data from a CSV file
*
*  Erik Bertse
*  Uppsala University
*
*/

package eb;
import java.io.*;
import java.util.*;

public class CSVReader {
	private ArrayList<ArrayList<String>> fullFile;
	private Map<Integer, String> keyColumnMap;
	private String filepath;
	private int numOfRows;
	
	
	/**
	* Constructor
	* @param f  the file path to the file to be opened.
	*/

	public CSVReader(String f) {

		this.fullFile = new ArrayList<>();
		this.keyColumnMap = new HashMap<>();
		this.filepath = f;

		try {
			BufferedReader br = new BufferedReader(
    			new InputStreamReader(
        			new FileInputStream(filepath),
        			"UTF-8"));
			

			String line = br.readLine();
			String[] splitLine = line.split("\t",-1);

			//We remove the BOM (byte order mark) in the first string.
			splitLine[0] = splitLine[0].replace("\ufeff", "");

			for (int i = 0; i<splitLine.length; i++) {
				keyColumnMap.put(i, splitLine[i]);
			}

			line = br.readLine();
			while (line != null) {

				ArrayList<String> entries = new ArrayList<>(Arrays.asList(line.split("\t",-1)));

				for (String s: entries) {
					s.trim();
				}

				fullFile.add(entries);
				line = br.readLine();

			}

			br.close();

		} catch (IOException e) { 
			System.out.println("IOException");
		}
		
		this.numOfRows = fullFile.size(); 
		

		//Prints the columns found in the file.
		System.out.println(" xxx Columns present in file: xxx \n");
		for (Map.Entry<Integer, String> entry : keyColumnMap.entrySet()) {
			System.out.printf ("  %-5s %s \n", entry.getKey(),  entry.getValue(), "st");
		}

		System.out.println(" \n xxxxxxxxxxxxxxxxxxx \n");
		
	}
	
	/**
	* Returns a particular string in the file.
	*
	* @param i  The row to get
	* @param j  The column to get
	* @return   The string at row i and column j
	*/
	public String get(int i, int j) {
		return fullFile.get(i).get(j);
	}

	/**
	* Returns the number of rows in the file.
	*
	* @return   The number of rows
	*/
	public int size() {
		return numOfRows;
	}

	/**
	* Returns the name of a column, given its number.
	*
	* @param colnum  the number of the column
	* @return        the name of the column
	*/
	public String nameOf(int colnum) {
		return keyColumnMap.get(colnum);
	}		

	/**
	* Counts the number of times a string appears in a column.
	*
	* @param colnum  the number of the column
	* @param s       the string to count.
	* @return        the number of times s appears in column colnum.
	*/
	public int count(int colnum, String s) {
		return count(colnum, 0, numOfRows, s);
	}

	/**
	* Counts the number of times a string appears in a column.
	* Only counts from row = low to row = high - 1.
	*
	* @param colnum  the number of the column
	* @param low     the lower bound
	* @param high    the higher bound
	* @param s       the string to count.
	* @return        the number of times s appears in column colnum.
	*/

	public int count(int colnum, int low, int high, String s) {
		int output = 0;

		for (int i = numOfRows-1; i >= 0; i--) {
			if (i >= low && i < high) {
				if (fullFile.get(i).get(colnum).equals(s) && !fullFile.get(i).get(colnum).isEmpty()) {
					output++;
				} 
			}
		}

		return output;	

	}

	/**
	* Counts the number of times a certain column is empty.
	*
	* @param colnum  the number of the column
	* @return        The number of times the value in the column is empty.
	*/
	public int countEmpty(int colnum) {
		int output = 0;

		for (int i = numOfRows-1; i >= 0; i--) {
			
			if (fullFile.get(i).get(colnum).isEmpty()) {
				output++;
			} 
		}
		return output;
	}


	/**
	* toString
	*
	* It is not advised to use this function in its current state, the printout is very messy.
	*
	* @return        String representation of the file.
	*/
	public String toString() {

		for (ArrayList<String> entries : fullFile) {

			for (String s : entries) {
				System.out.print(s + " | ");
			}
			System.out.print("\n");

		} 
		return "";
	}

	/**
	* Prints the contents of a column
	*	
	* @param colnum  the number of the column
	*/
	
	public void printColumn(int colnum) {
		printColumn(colnum, 0, numOfRows);
	}

	

	/**
	* Prints the contents of a column from row = low to row = high - 1
	*	
	* @param colnum  the number of the column
	* @param low     the lower bound
	* @param high    the higher bound
	*/
	public void printColumn(int colnum, int low, int high) {

		System.out.print("Printing column: " + colnum + " with title: " + keyColumnMap.get(colnum) + "\n \n");

		if (high > fullFile.size()) {
			high = fullFile.size();
		}

		for (int i = low; i < high; i++) { 
			ArrayList<String> entries = fullFile.get(i);
				
			System.out.print(entries.get(colnum));
			System.out.print("\n");

		} 
		System.out.print("\n");
	}


	/**
	* Prints the contents of several columns
	*	
	* @param cols    An array of indeces that are to be printed.
	*/	
	public void printColumns(int[] cols) {
		printColumns(cols, 0, numOfRows);
	}

	/**
	* Prints the contents of a column from row = low to row = high - 1
	*	
	* @param cols    An array of indeces that are to be printed.
	* @param low     the lower bound
	* @param high    the higher bound
	*/
	public void printColumns(int[] cols, int low, int high) {

		for (int i = low; i < high; i++) { 
			ArrayList<String> entries = fullFile.get(i);

			for (int j = 0; j<cols.length; j++) {

				if (j == cols.length - 1) {
					System.out.print(entries.get( cols[j] ));
				} else {
					System.out.print(entries.get(cols[j])  + " ::: " );
				}
				
			}
			System.out.print("\n");
		} 
		System.out.print("\n");
	}
	
	
	/**
	* Sorts the whole file in increasing order, by the given column.
	*	
	* @param colnum    The column to sort by.
	* 
	*/
	public void sortByColumn(int colnum) {
		sortByColumn(colnum, 0, numOfRows, 0);
	}


	/**
	* Sorts the whole file in increasing order, by the given column.
	*	
	* @param colnum    The column to sort by.
	* @param low       the lower bound
	* @param high      the higher bound
	* @param ascdesc   if this is 0, then we sort ascending, if this is 1, we sort descending. 
	*/
	public void sortByColumn(int colnum, int low, int high, int ascdesc) {
		
		if (high > numOfRows)
			high = numOfRows;

        boolean sortAsFloat = true;
        float[] floats = new float[numOfRows];
        String[] strings = new String[numOfRows];

        for (int i = 0; i<numOfRows; i++) {
        	if (i >= low && i < high) {
        		/* For each element, try matching with the Regex below.
        		*  The Regex says: First at least one digit, then maybe a decimal point, then arbitrarily many digits, and lastly maybe a percent sign.
        		*  If you fail at some element, the some element cannot be parsed as a float.
        		*/
        		if (!fullFile.get(i).get(colnum).matches("^(\\d)+.?(\\d)*%?$|()")) {
        			sortAsFloat = false;
        			break;
        		} 
        	} 
        }


        if (sortAsFloat) {
        	System.out.println("\n (xxx : Sorting column \"" + keyColumnMap.get(colnum) + "\" by floating point value) \n");
        } else {
        	System.out.println("\n (xxx : Sorting column \"" + keyColumnMap.get(colnum) + "\" by Ascii value) \n");
        }
        //The "high-1" is correct here, MergeSort treats the high value as an inclusive value.
        mergeSort(fullFile, colnum, low, high-1, ascdesc, sortAsFloat);

	}

	

    /**
	* Private function - MergeSort implementation.
	*
	* @param ff           The arrayList of Arraylists to be sorted	
	* @param colnum       The column to sort by.
	* @param low          the lower bound
	* @param high         the higher bound
	* @param ascdesc      if this is 0, then we sort ascending, if this is 1, we sort descending. 
	* @param sortAsFloat  If true, we sort by float value, otherwise by ascii value.
	*/
    private void mergeSort(ArrayList<ArrayList<String>> ff, int colnum, int low, int high, int ascdesc, boolean sortAsFloat) {
		if (low < high) {
			//find the middle value
			int m = (low + high)/2;

			//Sort the sublists
			mergeSort(ff, colnum, low, m, ascdesc, sortAsFloat);
			mergeSort(ff, colnum, m+1, high, ascdesc, sortAsFloat);
			
			//Merge the resulting halves.
			merge(ff, colnum, low, m, high, ascdesc, sortAsFloat);
		}
    }


    /**
	* Private function - merges two parts of an ArrayList.
	*
	* @param ff           The arrayList of Arraylists to be sorted	
	* @param colnum       The column to sort by.
	* @param low          the lower bound
	* @param middle       the middle index
	* @param high         the higher bound
	* @param ascdesc      if this is 0, then we sort ascending, if this is 1, we sort descending. 
	* @param sortAsFloat  If true, we sort by float value, otherwise by ascii value.
	*/
    private void merge(ArrayList<ArrayList<String>> ff, int colnum, int low, int middle, int high, int ascdesc, boolean sortAsFloat) {
    	//Highest index for bottom and top part.
    	int bottomIdx = middle - low + 1;
    	int topIdx = high - middle;
		
		//Temporary arraylists for each part.
		ArrayList<ArrayList<String>> lowPart = new ArrayList<>();
		ArrayList<ArrayList<String>> highPart = new ArrayList<>();

		//Populate the parts.
    	for (int i=0; i<bottomIdx; i++)
    		lowPart.add(ff.get(low + i));

        for (int j=0; j<topIdx; j++)
        	highPart.add(ff.get(middle + 1+ j));


        //We start inserting into the list at k = low.
    	int i = 0;
    	int j = 0;
    	int k = low;

    	while (i < bottomIdx && j < topIdx) {

    		String lowString;
    		String highString;

    		/*To know how to sort, we need to know what condition each pair of products 
    		* needs to satisfy (Ascii or numeric, ascending or descending) */
    		boolean condition;

    		if (sortAsFloat) {

    			lowString = lowPart.get(i).get(colnum).replace("%", "");
    			highString = highPart.get(j).get(colnum).replace("%", "");

    			if (ascdesc == 0) {
    				condition = Float.parseFloat(lowString) <= Float.parseFloat(highString);
    			} else {
    				condition = Float.parseFloat(lowString) >= Float.parseFloat(highString);
    			}
    		} else {

    			lowString = lowPart.get(i).get(colnum);
    			highString = highPart.get(j).get(colnum);

    			if (ascdesc == 0) {
    				condition = (lowString.compareTo(highString) <= 0);
    			} else {
    				condition = (lowString.compareTo(highString) >= 0);
    			}
    		}

    		// We take the appropriate element from each part and put it back.
    		if (condition) {
                ff.set(k, lowPart.get(i));
                i++;
            } else {
                ff.set(k, highPart.get(j));
                j++;
            }
            k++;
    	}

    	/* Copy remaining elements of the parts */
    	while (i < bottomIdx) {
            ff.set(k, lowPart.get(i));
            i++;
            k++;
        }
 
        
        while (j < topIdx) {
            ff.set(k, highPart.get(j));
            j++;
            k++;
        }

    }

}
//EOF