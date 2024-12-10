using Microsoft.AspNetCore.Mvc;

namespace WebApplication3.Controllers
{
	[Route("[controller]")]
	public class SoundBarController : Controller
    {
        public class VolMsg
        {
            public int? Id { get; set; }
            public int? Vol { get; set; }
        }
        public class AppId
        {
            public int? Id { get; set; }
            public string? Name { get; set; } = null;
            public int? Vol { get; set; }
            public AppId(int _id,string _name, int _vol=50)
            {
                Id = _id;
                Name = _name;
                Vol = _vol;
            }
        }

        [HttpGet("SoundBar")]
        public IActionResult SoundBar()
        {
            List<AppId> apps = new List<AppId>();
            apps.Add(new AppId(1, "Firefox",10));
            apps.Add(new AppId(2, "Brave",70));
            apps.Add(new AppId(3, "Discord"));
            return View(apps);
        }

		[HttpPost("ChangeVolume")]
		public IActionResult ChangeVolume([FromBody] List<VolMsg> msgs)
		{
            
            //Console.Write("INPUT");
            foreach (VolMsg volMsg in msgs)
            {
                Console.WriteLine(volMsg.Id +":"+ volMsg.Vol);
            }
			return Ok();
		}
	}
}
