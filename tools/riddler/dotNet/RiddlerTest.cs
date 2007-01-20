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