#include <SFGUI/Engines/BREW.hpp>
#include <SFGUI/ScrolledWindow.hpp>

namespace sfg {
namespace eng {

RenderQueue* BREW::CreateScrolledWindowDrawable( SharedPtr<const ScrolledWindow> scrolled_window ) const {
	sf::Color border_color_light( GetProperty<sf::Color>( "BorderColor", scrolled_window ) );
	sf::Color border_color_dark( GetProperty<sf::Color>( "BorderColor", scrolled_window ) );
	int border_color_shift( GetProperty<int>( "BorderColorShift", scrolled_window ) );
	float border_width( GetProperty<float>( "BorderWidth", scrolled_window ) );

	ShiftBorderColors( border_color_light, border_color_dark, border_color_shift );

	RenderQueue* queue( new RenderQueue );

	sf::FloatRect rect = scrolled_window->GetContentAllocation();

	rect.Left -= ( border_width + scrolled_window->GetAllocation().Left );
	rect.Top -= ( border_width + scrolled_window->GetAllocation().Top );
	rect.Width += 2.f * border_width;
	rect.Height += 2.f * border_width;

	queue->Add(
		CreateBorder(
			sf::FloatRect(
				0.f,
				0.f,
				rect.Width,
				rect.Height
			),
			border_width,
			border_color_dark,
			border_color_light
		)
	);

	return queue;
}

}
}
