using Microsoft.AspNetCore.Http.HttpResults;
using Microsoft.AspNetCore.Mvc;
using WebApplication3.Dependencies;
using System.Text.Json;

namespace WebApplication3.Controllers
{
	[Route("[controller]")]
	public class SoundBarController : Controller
    {
        private readonly AudioSessionMenager _audioManager;

        public SoundBarController(AudioSessionMenager audioManager)
        {
            _audioManager = audioManager;
        }

        public class VolMsg
        {
            public int Id { get; set; }
            public int Vol { get; set; }
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
            for (int i =0; i< _audioManager.SessionCount; i++)
            {
                apps.Add(new AppId(i, _audioManager.GetProcessName(i), (int)(_audioManager.GetVolume(i)*100)));
            }
            
            //apps.Add(new AppId(1, "Firefox",10));
            //apps.Add(new AppId(2, "Brave",70));
            //apps.Add(new AppId(3, "Discord"));
            return View(apps);
        }

		[HttpPost("ChangeVolume")]
		public IActionResult ChangeVolume([FromBody] List<VolMsg> msgs)
		{
            
            //Console.Write("INPUT");
            foreach (VolMsg volMsg in msgs)
            {
                //Console.WriteLine(volMsg.Id +":"+ volMsg.Vol);
                _audioManager.SetVolume(volMsg.Id, ((float)volMsg.Vol / 100));
            }
			return Ok();
		}

        [Route("icon/{index}")]
        public IActionResult GetProcessIcon(int index)
        {
            byte[] iconBytes = _audioManager.GetIconBytes(index);
            if (iconBytes == null)
                return NotFound();

            return File(iconBytes, "image/png");
        }

        [HttpGet("UpdateSessions")]
        public IActionResult UpdateSessions()
        {
            _audioManager.UpdateSessions();
            List<AppId> apps = new List<AppId>();
            for (int i = 0; i < _audioManager.SessionCount; i++)
            {
                apps.Add(new AppId(i, _audioManager.GetProcessName(i), (int)(_audioManager.GetVolume(i) * 100)));
            }
            
            var appsJson = Json(JsonSerializer.Serialize(apps));

            return appsJson;
        }
    }
}
