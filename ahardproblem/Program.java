/**
 *  A hard problem.
 *
 *  This code was written as a solution to the following puzzle:
 *  
 *    You are to construct a walled garden. The garden's design can be irregularly shaped, 
 *    but it MUST contain certain plants within its walls, whose
 *    locations is provided to you as x and y-coordinates. You are to build shortest 
 *    possible wall that contain all the necessary plants.
 *
 *    The garden wall merchant is unfortunately only selling pieces of wall of predefined length
 *    and price. 
 *
 *    Given a set of plants, as specified by their x and y - coordinates,
 *    and a set of pieces of wall, as specified by their length in meters m, and 
 *    their price p, whats the cheapest price we could get away with to construct the whole 
 *    wall around the garden, i.e.
 *    
 *    Whats the lowest possible p such that the sum of m is greater than the circumference of the convex hull
 *    formed by the plants?
 *
 *  The solver class contains all the necessary functionality.
 *  See the solver class for solution description.
 *  
 *  This class does not validate the input in any way. The code assumes more than 3 points,
 *  and that these are not all colinear.
 *  
 *  Extremely fast, even for large inputs.
 * 
 *  Written by Erik Bertse
 */

import java.io.IOException;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.List;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.Collections;
import java.util.Stack;

public class Program {

    public static void main(String[] args) {

    	
        List<Point> points = new ArrayList<Point>();
        int [] meters = null; 
        int [] prices = null; 
        
        Solver solver = new Solver();

        BufferedReader br = new BufferedReader(new InputStreamReader(System.in));

        try {
            System.out.println("Please provide the number of walls: ");
            int noWalls = Integer.parseInt(br.readLine());
            
            System.out.println("Please provide the number of points: ");
            int noPoints = Integer.parseInt(br.readLine());
    
            // intexing from 1, to conform with grahamScan
            meters = new int[noWalls+1];
            prices = new int[noWalls+1];
    
            System.out.println("Please provide " + noWalls + " pairs (m,p) of integer separated by a space "); 
            for (int i = 0; i< noWalls; i++) {
    
                String line = br.readLine();
                String[] pair = line.split(" ");

                meters[i+1] = Integer.parseInt(pair[0]);
                prices[i+1] = Integer.parseInt(pair[1]);
            }
                
            System.out.println("Please provide " + noPoints + " pairs (x,y) of integers separated by a space"); 
            for (int i = 0; i< noPoints; i++) {

                String line = br.readLine();
                String[] pair = line.split(" ");

                int x = Integer.parseInt(pair[0]);
                int y = Integer.parseInt(pair[1]);
                points.add(new Point(x, y));
            
            }
                
            double wallLength = solver.grahamScan(points);
            int minPrice = solver.minimumKnapSack(meters, prices, wallLength);
            System.out.println("Smallest possible price is: " + minPrice); 
        
        } catch (IOException e) { }

    }

}
