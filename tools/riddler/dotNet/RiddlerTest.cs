#region Copyright (c) 2002, James W. Newkirk, Michael C. Two, Alexei A. Vorontsov, Philip A. Craig
/************************************************************************************
'
' Copyright  2002 James W. Newkirk, Michael C. Two, Alexei A. Vorontsov
' Copyright  2000-2002 Philip A. Craig
'
' This software is provided 'as-is', without any express or implied warranty. In no 
' event will the authors be held liable for any damages arising from the use of this 
' software.
' 
' Permission is granted to anyone to use this software for any purpose, including 
' commercial applications, and to alter it and redistribute it freely, subject to the 
' following restrictions:
'
' 1. The origin of this software must not be misrepresented; you must not claim that 
' you wrote the original software. If you use this software in a product, an 
' acknowledgment (see the following) in the product documentation is required.
'
' Portions Copyright  2002 James W. Newkirk, Michael C. Two, Alexei A. Vorontsov 
' or Copyright  2000-2002 Philip A. Craig
'
' 2. Altered source versions must be plainly marked as such, and must not be 
' misrepresented as being the original software.
'
' 3. This notice may not be removed or altered from any source distribution.
'
'***********************************************************************************/
#endregion

namespace edu.cmu.sphinx.riddler
{
	using System;
	using NUnit.Framework;   

	/// <summary>NUnit test suite for Sphinx Riddler web services</summary>
	/// 
	[TestFixture] 
	public class RiddlerTest
	{
        /// <summary>
        /// Riddler reference itself
        /// </summary>
        protected Riddler riddler;
        /// <summary>
		/// 
		/// </summary>
		protected string string1;
		
		/// <summary>
		/// 
		/// </summary>
		[SetUp] public void Init() 
		{
            riddler = new Riddler();
            string1 = "a predefined String";			
		}

		/// <summary>
		/// 
		/// </summary>
		///
		[Test] public void Echo() 
		{
            string result = riddler.echo(string1);
			Assert.AreEqual(string1, result, "Input and result are equal");
		}

		[Test]
		[ExpectedException(typeof(InvalidOperationException))]
		public void ExpectAnException()
		{
			throw new InvalidCastException();
		}

		[Test]
		[Ignore("ignored test")]
		public void IgnoredTest()
		{
			throw new Exception();
		}
	}
}